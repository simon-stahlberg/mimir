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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/grounded.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/state_repository.hpp"

#include <deque>
#include <memory>

namespace mimir
{

/// @brief Compute a ground atom order where ground atoms over same predicate are next to each other
/// The basic idea is that those can be potential mutex variables and grouping them together
/// can result in a smaller match tree. Such math tree structures have size linear in the number of mutex variables.
/// We also consider larger groups first since such mutex variables would result in a very large linear split.
template<PredicateTag P>
static std::vector<size_t> compute_ground_atom_order(const GroundAtomList<P>& atoms, const PDDLRepositories& pddl_repositories)
{
    auto ground_atoms_order = std::vector<size_t> {};
    auto m_ground_atoms_by_predicate = std::unordered_map<Predicate<P>, GroundAtomList<P>> {};
    for (const auto& ground_atom : atoms)
    {
        m_ground_atoms_by_predicate[ground_atom->get_predicate()].push_back(ground_atom);
    }
    auto ground_atoms = GroundAtomList<P> {};
    // Sort group decreasingly in their size, break ties lexicographically by predicate name
    auto sorted_groups = std::vector<std::pair<Predicate<P>, GroundAtomList<P>>> {};
    for (const auto& [predicate, group] : m_ground_atoms_by_predicate)
    {
        sorted_groups.emplace_back(predicate, group);
    }
    std::sort(sorted_groups.begin(),
              sorted_groups.end(),
              [](const std::pair<Predicate<P>, GroundAtomList<P>>& left, const std::pair<Predicate<P>, GroundAtomList<P>>& right)
              {
                  if (left.second.size() == right.second.size())
                  {
                      return left.first->get_name() < right.first->get_name();
                  }
                  return left.second.size() > right.second.size();
              });
    for (const auto& [_predicate, group] : sorted_groups)
    {
        // Sort grounded atoms in each group by indices to get compiler independent results.
        auto sorted_group = group;
        std::sort(sorted_group.begin(), sorted_group.end(), [](const GroundAtom<P>& left, const GroundAtom<P>& right) { return left->str() < right->str(); });
        for (const auto& grounded_atom : sorted_group)

        {
            ground_atoms_order.push_back(grounded_atom->get_index());
        }
    }
    return ground_atoms_order;
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    GroundedApplicableActionGenerator(problem, std::move(pddl_repositories), std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem,
                                                                     std::shared_ptr<PDDLRepositories> pddl_repositories,
                                                                     std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_event_handler(std::move(event_handler)),
    m_lifted_applicable_action_generator(m_problem, m_pddl_repositories)
{
    /* 1. Explore delete relaxed task. We explicitly require to keep actions and axioms with empty effects. */
    auto delete_relax_transformer = DeleteRelaxTransformer(*m_pddl_repositories, false);
    // TODO: In the case of positive normal form, we want this problem
    // to be modified such that grounded actions have no negative preconditions.
    // We also need access to the dual predicates to set the polarity of the initial literals.
    // auto to_pnf_grounded_transformer = ToPositiveNormalFormGroundTransformer(m_pddl_repositories);
    // m_problem = to_pnf_grounded_transformer.run(*m_problem);
    const auto delete_free_problem = delete_relax_transformer.run(*m_problem);
    auto delete_free_lifted_applicable_action_generator = std::make_shared<LiftedApplicableActionGenerator>(delete_free_problem, m_pddl_repositories);
    auto delete_free_state_repository = StateRepository(delete_free_lifted_applicable_action_generator);

    auto state_builder = StateImpl();
    auto state_builder_tmp = StateImpl();
    auto& fluent_state_atoms = state_builder.get_atoms<Fluent>();
    auto& derived_state_atoms = state_builder.get_atoms<Derived>();
    const auto initial_state = delete_free_state_repository.get_or_create_initial_state();
    fluent_state_atoms = initial_state->get_atoms<Fluent>();
    derived_state_atoms = initial_state->get_atoms<Derived>();

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    // Serialization buffer
    cista::buf<std::vector<uint8_t>> state_buf;

    auto actions = GroundActionList {};
    do
    {
        reached_delete_free_explore_fixpoint = true;

        state_builder_tmp = state_builder;
        const auto state = state_builder_tmp;

        auto num_atoms_before = fluent_state_atoms.count();

        // Create and all applicable actions and apply them
        // Attention: we cannot just apply newly generated actions because conditional effects might trigger later.
        delete_free_lifted_applicable_action_generator->generate_applicable_actions(&state, actions);
        for (const auto& action : actions)
        {
            const auto succ_state = delete_free_state_repository.get_or_create_successor_state(&state, action);
            for (const auto atom_index : succ_state->get_atoms<Fluent>())
            {
                fluent_state_atoms.set(atom_index);
            }
        }

        // Create and all applicable axioms and apply them
        delete_free_lifted_applicable_action_generator->generate_and_apply_axioms(state_builder);

        auto num_atoms_after = fluent_state_atoms.count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);

    m_event_handler->on_finish_delete_free_exploration(m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(fluent_state_atoms),
                                                       m_pddl_repositories->get_ground_atoms_from_indices<Derived>(derived_state_atoms),
                                                       delete_free_lifted_applicable_action_generator->get_ground_actions(),
                                                       delete_free_lifted_applicable_action_generator->get_ground_axioms());

    auto fluent_ground_atoms_order =
        compute_ground_atom_order(m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(fluent_state_atoms), *m_pddl_repositories);
    auto derived_ground_atoms_order =
        compute_ground_atom_order(m_pddl_repositories->get_ground_atoms_from_indices<Derived>(derived_state_atoms), *m_pddl_repositories);

    // 2. Create ground actions
    /* TODO: we want ground actions in PNF and problem with modified initial state.
       Ok, we got the relaxed reachable initial atoms.
       We still need modified lifted actions for grounding.
     */
    auto ground_actions = GroundActionList {};
    for (const auto& action : delete_free_lifted_applicable_action_generator->get_ground_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_action : delete_relax_transformer.get_unrelaxed_actions(m_pddl_repositories->get_action(action->get_action_index())))
        {
            auto action_arguments = m_pddl_repositories->get_objects_from_indices(action->get_object_indices());
            auto grounded_action = m_lifted_applicable_action_generator.ground_action(unrelaxed_action, std::move(action_arguments));
            if (grounded_action->is_statically_applicable(problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_actions.push_back(grounded_action);
            }
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_actions(ground_actions);

    // 3. Build match tree
    m_action_match_tree = MatchTree(ground_actions, fluent_ground_atoms_order, derived_ground_atoms_order);

    m_event_handler->on_finish_build_action_match_tree(m_action_match_tree);

    // 2. Create ground axioms
    auto ground_axioms = GroundAxiomList {};
    for (const auto& axiom : delete_free_lifted_applicable_action_generator->get_ground_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_axiom : delete_relax_transformer.get_unrelaxed_axioms(m_pddl_repositories->get_axiom(axiom->get_axiom_index())))
        {
            auto axiom_arguments = m_pddl_repositories->get_objects_from_indices(axiom->get_objects());
            auto grounded_axiom = m_lifted_applicable_action_generator.ground_axiom(unrelaxed_axiom, std::move(axiom_arguments));
            if (grounded_axiom->is_statically_applicable(problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_axioms.push_back(grounded_axiom);
            }
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_axioms(ground_axioms);

    // 3. Build match tree
    m_axiom_match_tree = MatchTree(ground_axioms, fluent_ground_atoms_order, derived_ground_atoms_order);

    m_event_handler->on_finish_build_axiom_match_tree(m_axiom_match_tree);
}

void GroundedApplicableActionGenerator::generate_applicable_actions(State state, GroundActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_action_match_tree.get_applicable_elements(state->get_atoms<Fluent>(), state->get_atoms<Derived>(), out_applicable_actions);
}

void GroundedApplicableActionGenerator::generate_and_apply_axioms(StateImpl& unextended_state)
{
    for (const auto& lifted_partition : m_lifted_applicable_action_generator.get_axiom_partitioning())
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            auto applicable_axioms = GroundAxiomList {};

            // TODO: For axioms, the same fluent branch is taken all the time.
            // Exploit this!
            m_axiom_match_tree.get_applicable_elements(unextended_state.get_atoms<Fluent>(), unextended_state.get_atoms<Derived>(), applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                if (!lifted_partition.get_axioms().count(m_pddl_repositories->get_axiom(grounded_axiom->get_axiom_index())))
                {
                    // axiom not part of same partition
                    continue;
                }

                assert(!grounded_axiom->get_derived_effect().is_negated);

                const auto grounded_atom_index = grounded_axiom->get_derived_effect().atom_index;

                if (!unextended_state.get_atoms<Derived>().get(grounded_atom_index))
                {
                    // GENERATED NEW DERIVED ATOM!
                    reached_partition_fixed_point = false;
                }

                unextended_state.get_atoms<Derived>().set(grounded_atom_index);
            }

        } while (!reached_partition_fixed_point);
    }
}

void GroundedApplicableActionGenerator::on_finish_search_layer() const { m_event_handler->on_finish_search_layer(); }

void GroundedApplicableActionGenerator::on_end_search() const { m_event_handler->on_end_search(); }

const GroundActionList& GroundedApplicableActionGenerator::get_ground_actions() const { return m_lifted_applicable_action_generator.get_ground_actions(); }

GroundAction GroundedApplicableActionGenerator::get_ground_action(Index action_index) const
{
    return m_lifted_applicable_action_generator.get_ground_action(action_index);
}

const GroundAxiomList& GroundedApplicableActionGenerator::get_ground_axioms() const { return m_lifted_applicable_action_generator.get_ground_axioms(); }

GroundAxiom GroundedApplicableActionGenerator::get_ground_axiom(Index axiom_index) const
{
    return m_lifted_applicable_action_generator.get_ground_axiom(axiom_index);
}

size_t GroundedApplicableActionGenerator::get_num_ground_actions() const { return m_lifted_applicable_action_generator.get_num_ground_actions(); }

size_t GroundedApplicableActionGenerator::get_num_ground_axioms() const { return m_lifted_applicable_action_generator.get_num_ground_axioms(); }

Problem GroundedApplicableActionGenerator::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& GroundedApplicableActionGenerator::get_pddl_repositories() const { return m_pddl_repositories; }

}
