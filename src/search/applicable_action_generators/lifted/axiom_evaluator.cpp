/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/lifted/axiom_evaluator.hpp"

#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_tag.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/condition_grounders.hpp"

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace mimir
{

void AxiomEvaluator::generate_and_apply_axioms(StateImpl& unextended_state)
{
    /* 1. Initialize assignment set */

    m_event_handler->on_start_generating_applicable_axioms();

    // TODO: In principle, we could reuse the resulting assignment set from the lifted AAG but it is difficult to access here.
    const auto fluent_assignment_set = AssignmentSet<Fluent>(m_problem,
                                                             m_problem->get_domain()->get_predicates<Fluent>(),
                                                             m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(unextended_state.get_atoms<Fluent>()));

    auto derived_assignment_set = AssignmentSet<Derived>(m_problem,
                                                         m_problem->get_problem_and_domain_derived_predicates(),
                                                         m_pddl_repositories->get_ground_atoms_from_indices<Derived>(unextended_state.get_atoms<Derived>()));

    /* 2. Fixed point computation */

    auto bindings = std::vector<ObjectList> {};
    auto applicable_axioms = GroundAxiomList {};

    for (const auto& partition : m_partitioning)
    {
        bool reached_partition_fixed_point;

        // Optimization 1: Track new ground atoms to simplify the clique enumeration graph in the next iteration.
        auto new_ground_atoms = GroundAtomList<Derived> {};

        // TODO: Optimization 4: Inductively compile away axioms with static bodies. (grounded in match tree?)

        // Optimization 2: Track axioms that might trigger in next iteration.
        // Optimization 3: Use precomputed set of axioms that might be applicable initially
        auto relevant_axioms = partition.get_initially_relevant_axioms();

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms */

            applicable_axioms.clear();
            for (const auto& axiom : relevant_axioms)
            {
                auto& condition_grounder = m_condition_grounders.at(axiom);
                condition_grounder.compute_bindings(&unextended_state, fluent_assignment_set, derived_assignment_set, bindings);

                for (auto& binding : bindings)
                {
                    applicable_axioms.emplace_back(ground_axiom(axiom, std::move(binding)));
                }
            }

            /* Apply applicable axioms */

            new_ground_atoms.clear();
            relevant_axioms.clear();

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(!grounded_axiom->get_derived_effect().is_negated);

                assert(grounded_axiom->is_applicable(unextended_state.get_atoms<Fluent>(),
                                                     unextended_state.get_atoms<Derived>(),
                                                     m_problem->get_static_initial_positive_atoms_bitset()));

                const auto grounded_atom_index = grounded_axiom->get_derived_effect().atom_index;

                if (!unextended_state.get_atoms<Derived>().get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    const auto new_ground_atom = m_pddl_repositories->get_ground_atom<Derived>(grounded_atom_index);
                    reached_partition_fixed_point = false;

                    // TODO: Optimization 5: Update new ground atoms to speed up successive iterations, i.e.,
                    // only cliques that takes these new atoms into account must be computed.
                    new_ground_atoms.push_back(new_ground_atom);

                    // Update the assignment set
                    derived_assignment_set.insert_ground_atom(new_ground_atom);

                    // Retrieve relevant axioms
                    partition.retrieve_axioms_with_same_body_predicate(new_ground_atom, relevant_axioms);
                }

                unextended_state.get_atoms<Derived>().set(grounded_atom_index);
            }

        } while (!reached_partition_fixed_point);
    }

    m_event_handler->on_end_generating_applicable_axioms(applicable_axioms, *m_pddl_repositories);
}

AxiomEvaluator::AxiomEvaluator(Problem problem,
                               std::shared_ptr<PDDLRepositories> pddl_repositories,
                               std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_event_handler(std::move(event_handler)),
    m_partitioning(),
    m_flat_axioms(),
    m_axioms_by_index(),
    m_axiom_builder(),
    m_axiom_groundings(),
    m_condition_grounders()

{
    /* 1. Error checking */

    for (const auto& literal : problem->get_fluent_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& literal : problem->get_static_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    /* 2. Initialize axiom partitioning using stratification */

    auto axioms = m_problem->get_domain()->get_axioms();
    axioms.insert(axioms.end(), m_problem->get_axioms().begin(), m_problem->get_axioms().end());

    for (const auto& axiom : axioms)
    {
        if (axiom->get_literal()->is_negated())
        {
            throw std::runtime_error("Negative literals in axiom heads is not supported.");
        }
    }

    auto derived_predicates = m_problem->get_domain()->get_predicates<Derived>();
    derived_predicates.insert(derived_predicates.end(), m_problem->get_derived_predicates().begin(), m_problem->get_derived_predicates().end());

    m_partitioning = compute_axiom_partitioning(axioms, derived_predicates);

    /* 3. Initialize condition grounders */

    auto static_initial_atoms = GroundAtomList<Static> {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = AssignmentSet<Static>(m_problem, m_problem->get_domain()->get_predicates<Static>(), static_initial_atoms);

    for (const auto& axiom : axioms)
    {
        m_condition_grounders.emplace(axiom,
                                      ConditionGrounder(m_problem,
                                                        axiom->get_parameters(),
                                                        axiom->get_conditions<Static>(),
                                                        axiom->get_conditions<Fluent>(),
                                                        axiom->get_conditions<Derived>(),
                                                        static_assignment_set,
                                                        m_pddl_repositories));
    }
}

const std::vector<AxiomPartition>& AxiomEvaluator::get_axiom_partitioning() const { return m_partitioning; }

GroundAxiom AxiomEvaluator::ground_axiom(Axiom axiom, ObjectList&& binding)
{
    /* 1. Check if grounding is cached */

    auto& groundings = m_axiom_groundings[axiom];
    auto it = groundings.find(binding);
    if (it != groundings.end())
    {
        m_event_handler->on_ground_axiom_cache_hit(axiom, binding);

        return it->second;
    }

    m_event_handler->on_ground_axiom_cache_miss(axiom, binding);

    /* 2. Ground the axiom */

    m_event_handler->on_ground_axiom(axiom, binding);

    /* Header */

    m_axiom_builder.get_index() = m_flat_axioms.size();
    m_axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = m_axiom_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }

    /* Precondition */
    auto& strips_precondition = m_axiom_builder.get_strips_precondition();
    auto& positive_fluent_precondition = strips_precondition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = strips_precondition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = strips_precondition.get_positive_precondition<Static>();
    auto& negative_static_precondition = strips_precondition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = strips_precondition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = strips_precondition.get_negative_precondition<Derived>();
    positive_fluent_precondition.unset_all();
    negative_fluent_precondition.unset_all();
    positive_static_precondition.unset_all();
    negative_static_precondition.unset_all();
    positive_derived_precondition.unset_all();
    negative_derived_precondition.unset_all();
    m_pddl_repositories->ground_and_fill_bitset(axiom->get_conditions<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    m_pddl_repositories->ground_and_fill_bitset(axiom->get_conditions<Static>(), positive_static_precondition, negative_static_precondition, binding);
    m_pddl_repositories->ground_and_fill_bitset(axiom->get_conditions<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);

    /* Effect */

    // The effect literal might only use the first few objects of the complete binding
    // Therefore, we can prevent the literal grounding table from unnecessarily growing
    // by restricting the binding to only the relevant part
    const auto effect_literal_arity = axiom->get_literal()->get_atom()->get_arity();
    const auto is_complete_binding_relevant_for_head = (binding.size() == effect_literal_arity);
    const auto grounded_literal =
        is_complete_binding_relevant_for_head ?
            m_pddl_repositories->ground_literal(axiom->get_literal(), binding) :
            m_pddl_repositories->ground_literal(axiom->get_literal(), ObjectList(binding.begin(), binding.begin() + effect_literal_arity));
    assert(!grounded_literal->is_negated());
    m_axiom_builder.get_derived_effect().is_negated = false;
    m_axiom_builder.get_derived_effect().atom_index = grounded_literal->get_atom()->get_index();

    const auto [iter, inserted] = m_flat_axioms.insert(m_axiom_builder);
    const auto grounded_axiom = *iter;

    if (inserted)
    {
        m_axioms_by_index.push_back(grounded_axiom);
    }

    /* 3. Insert to groundings table */

    groundings.emplace(std::move(binding), GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

const GroundAxiomList& AxiomEvaluator::get_ground_axioms() const { return m_axioms_by_index; }

GroundAxiom AxiomEvaluator::get_ground_axiom(Index axiom_index) const { return m_axioms_by_index.at(axiom_index); }

size_t AxiomEvaluator::get_num_ground_axioms() const { return m_axioms_by_index.size(); }

}
