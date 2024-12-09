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

#include "mimir/search/delete_relaxed_problem_explorator.hpp"

#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/axiom_evaluators/lifted.hpp"
#include "mimir/search/grounding/action_grounder.hpp"
#include "mimir/search/grounding/axiom_grounder.hpp"
#include "mimir/search/grounding/match_tree.hpp"

namespace mimir
{

/// @brief Compute a ground atom order where ground atoms over same predicate are next to each other
/// The basic idea is that those can be potential mutex variables and grouping them together
/// can result in a smaller match tree. Such math tree structures have size linear in the number of mutex variables.
/// We also consider larger groups first since such mutex variables would result in a very large linear split.
template<PredicateTag P>
static std::vector<size_t> compute_ground_atom_order(const PDDLRepositories& pddl_repositories)
{
    auto ground_atoms_order = std::vector<size_t> {};
    auto m_ground_atoms_by_predicate = std::unordered_map<Predicate<P>, GroundAtomList<P>> {};
    for (const auto& ground_atom : pddl_repositories.get_ground_atoms<P>())
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
        // Sort grounded atoms in each group by names to get compiler independent results.
        auto sorted_group = group;
        std::sort(sorted_group.begin(),
                  sorted_group.end(),
                  [](const GroundAtom<P>& left, const GroundAtom<P>& right) { return to_string(*left) < to_string(*right); });
        for (const auto& grounded_atom : sorted_group)

        {
            ground_atoms_order.push_back(grounded_atom->get_index());
        }
    }
    return ground_atoms_order;
}

DeleteRelaxedProblemExplorator::DeleteRelaxedProblemExplorator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_delete_relax_transformer(*m_pddl_repositories, false),
    m_delete_free_problem(m_delete_relax_transformer.run(*m_problem)),
    m_delete_free_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_delete_free_problem, m_pddl_repositories)),
    m_delete_free_axiom_evalator(std::make_shared<LiftedAxiomEvaluator>(m_delete_free_problem, m_pddl_repositories)),
    m_delete_free_state_repository(StateRepository(std::static_pointer_cast<IAxiomEvaluator>(m_delete_free_axiom_evalator)))
{
    auto state_builder = StateImpl(*m_delete_free_state_repository.get_or_create_initial_state());
    auto& fluent_state_atoms = state_builder.get_atoms<Fluent>();

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    do
    {
        reached_delete_free_explore_fixpoint = true;

        auto num_atoms_before = fluent_state_atoms.count();

        const auto state_tmp = state_builder;

        // Create and all applicable actions and apply them
        // Attention: we cannot just apply newly generated actions because conditional effects might trigger later.
        for (const auto& action : m_delete_free_applicable_action_generator->create_applicable_action_generator(&state_tmp))
        {
            const auto [succ_state, action_cost] = m_delete_free_state_repository.get_or_create_successor_state(&state_builder, action);
            for (const auto atom_index : succ_state->get_atoms<Fluent>())
            {
                fluent_state_atoms.set(atom_index);
            }
        }

        // Create and all applicable axioms and apply them
        m_delete_free_axiom_evalator->generate_and_apply_axioms(state_builder);

        auto num_atoms_after = fluent_state_atoms.count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);
}

std::shared_ptr<GroundedAxiomEvaluator>
DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator(std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler) const
{
    event_handler->on_finish_delete_free_exploration(
        m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(m_delete_free_state_repository.get_reached_fluent_ground_atoms_bitset()),
        m_pddl_repositories->get_ground_atoms_from_indices<Derived>(m_delete_free_state_repository.get_reached_derived_ground_atoms_bitset()),
        m_delete_free_axiom_evalator->get_axiom_grounder().get_ground_axioms());

    auto axiom_grounder = AxiomGrounder(m_problem, m_pddl_repositories);

    auto ground_axioms = GroundAxiomList {};
    for (const auto& axiom : m_delete_free_axiom_evalator->get_axiom_grounder().get_ground_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_axiom : m_delete_relax_transformer.get_unrelaxed_axioms(m_pddl_repositories->get_axiom(axiom->get_axiom_index())))
        {
            auto axiom_arguments = m_pddl_repositories->get_objects_from_indices(axiom->get_object_indices());
            auto grounded_axiom = axiom_grounder.ground_axiom(unrelaxed_axiom, std::move(axiom_arguments));
            if (grounded_axiom->is_statically_applicable(m_problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_axioms.push_back(grounded_axiom);
            }
        }
    }

    event_handler->on_finish_grounding_unrelaxed_axioms(ground_axioms);

    // Compute order here after grounding all axioms. Could also be moved into the match tree.
    const auto fluent_atoms_ordering = compute_ground_atom_order<Fluent>(*m_pddl_repositories);
    const auto derived_atoms_ordering = compute_ground_atom_order<Derived>(*m_pddl_repositories);
    auto match_tree = MatchTree(ground_axioms, fluent_atoms_ordering, derived_atoms_ordering);

    event_handler->on_finish_build_axiom_match_tree(match_tree);

    return std::make_shared<GroundedAxiomEvaluator>(std::move(axiom_grounder), std::move(match_tree), std::move(event_handler));
}

std::shared_ptr<GroundedApplicableActionGenerator>
DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator(std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) const
{
    event_handler->on_finish_delete_free_exploration(
        m_pddl_repositories->get_ground_atoms_from_indices<Fluent>(m_delete_free_state_repository.get_reached_fluent_ground_atoms_bitset()),
        m_pddl_repositories->get_ground_atoms_from_indices<Derived>(m_delete_free_state_repository.get_reached_derived_ground_atoms_bitset()),
        m_delete_free_applicable_action_generator->get_action_grounder().get_ground_actions());

    auto action_grounder = ActionGrounder(m_problem, m_pddl_repositories);

    auto ground_actions = GroundActionList {};
    for (const auto& action : m_delete_free_applicable_action_generator->get_action_grounder().get_ground_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_action : m_delete_relax_transformer.get_unrelaxed_actions(m_pddl_repositories->get_action(action->get_action_index())))
        {
            auto action_arguments = m_pddl_repositories->get_objects_from_indices(action->get_object_indices());
            auto grounded_action = action_grounder.ground_action(unrelaxed_action, std::move(action_arguments));
            if (grounded_action->is_statically_applicable(m_problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_actions.push_back(grounded_action);
            }
        }
    }

    event_handler->on_finish_grounding_unrelaxed_actions(ground_actions);

    // Compute order here after grounding all actions. Could also be moved into the match tree.
    const auto fluent_atoms_ordering = compute_ground_atom_order<Fluent>(*m_pddl_repositories);
    const auto derived_atoms_ordering = compute_ground_atom_order<Derived>(*m_pddl_repositories);
    auto match_tree = MatchTree(ground_actions, fluent_atoms_ordering, derived_atoms_ordering);

    event_handler->on_finish_build_action_match_tree(match_tree);

    return std::make_shared<GroundedApplicableActionGenerator>(std::move(action_grounder), std::move(match_tree), std::move(event_handler));
}

Problem DeleteRelaxedProblemExplorator::get_problem() const { return m_problem; }
const std::shared_ptr<PDDLRepositories>& DeleteRelaxedProblemExplorator::get_pddl_repositories() const { return m_pddl_repositories; }

}  // namespace mimir
