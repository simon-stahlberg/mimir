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

#include "mimir/search/grounding/axiom_grounder.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/axiom.hpp"

namespace mimir
{
AxiomGrounder::AxiomGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(pddl_repositories),
    m_condition_grounders(),
    m_axioms(),
    m_axioms_by_index(),
    m_axiom_builder(),
    m_axiom_groundings()
{
    /* 3. Initialize condition grounders */

    auto static_initial_atoms = GroundAtomList<Static> {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = AssignmentSet<Static>(m_problem, m_problem->get_domain()->get_predicates<Static>(), static_initial_atoms);

    for (const auto& axiom : problem->get_problem_and_domain_axioms())
    {
        m_condition_grounders.emplace(axiom,
                                      ConditionGrounder(m_problem,
                                                        m_pddl_repositories,
                                                        axiom->get_parameters(),
                                                        axiom->get_conditions<Static>(),
                                                        axiom->get_conditions<Fluent>(),
                                                        axiom->get_conditions<Derived>(),
                                                        static_assignment_set));
    }
}

/// @brief Ground an axiom and return a view onto it.
GroundAxiom AxiomGrounder::ground_axiom(Axiom axiom, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& groundings = m_axiom_groundings[axiom];
    auto it = groundings.find(binding);
    if (it != groundings.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    /* Header */

    m_axiom_builder.get_index() = m_axioms.size();
    m_axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = m_axiom_builder.get_object_indices();
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

    const auto [iter, inserted] = m_axioms.insert(m_axiom_builder);
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

std::unordered_map<Axiom, ConditionGrounder>& AxiomGrounder::get_axiom_precondition_grounders() { return m_condition_grounders; }

/// @brief Return all axioms.
const GroundAxiomList& AxiomGrounder::get_ground_axioms() const { return m_axioms_by_index; }

GroundAxiom AxiomGrounder::get_ground_axiom(Index axiom_index) const { return m_axioms_by_index.at(axiom_index); }

size_t AxiomGrounder::get_num_ground_axioms() const { return m_axioms_by_index.size(); }

Problem AxiomGrounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& AxiomGrounder::get_pddl_repositories() const { return m_pddl_repositories; }
}
