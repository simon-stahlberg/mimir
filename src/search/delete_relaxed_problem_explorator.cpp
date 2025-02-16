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

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers.hpp"
#include "mimir/search/axiom_evaluators/lifted.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

namespace mimir
{

DeleteRelaxedProblemExplorator::DeleteRelaxedProblemExplorator(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_delete_relax_transformer(*m_pddl_repositories),  ///< We have to use input grounders pddl_repositories to map the relaxed actions/axioms
                                                       /// back to unrelaxed actions/axioms
    m_delete_free_problem(m_delete_relax_transformer.run(m_problem)),
    // m_delete_free_grounder(std::make_shared<Grounder>(m_delete_free_problem,
    //                                                   m_pddl_repositories)),  // important to instantiate the grounder for the delete-free problem!
    m_delete_free_applicable_action_generator(std::make_shared<LiftedApplicableActionGenerator>(m_problem, m_pddl_repositories)),
    m_delete_free_axiom_evalator(std::make_shared<LiftedAxiomEvaluator>(m_problem, m_pddl_repositories)),
    m_delete_free_state_repository(StateRepository(std::static_pointer_cast<IAxiomEvaluator>(m_delete_free_axiom_evalator)))
{
    std::cout << "[DeleteRelaxedProblemExplorator] Started delete relaxed exploration." << std::endl;

    const auto start_time = std::chrono::high_resolution_clock::now();

    auto initial_state = m_delete_free_state_repository.get_or_create_initial_state();

    auto dense_state = DenseState(initial_state);

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    do
    {
        reached_delete_free_explore_fixpoint = true;

        auto num_atoms_before = m_delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count();

        // Create and all applicable actions and apply them
        // Attention1: we cannot just apply newly generated actions because conditional effects might trigger later.
        // Attention2: we incrementally keep growing the atoms in the dense state.
        for (const auto& action : m_delete_free_applicable_action_generator->create_applicable_action_generator(dense_state))
        {
            // Note that get_or_create_successor_state already modifies dense_state to be the successor state.
            // TODO(numeric): in the delete relaxation, we have to remove all numeric constraints and effects.
            m_delete_free_state_repository.get_or_create_successor_state(dense_state, action, 0);
        }

        // Create and all applicable axioms and apply them
        m_delete_free_axiom_evalator->generate_and_apply_axioms(dense_state);

        // Note: checking fluent atoms suffices because derived are implied by those.
        auto num_atoms_after = m_delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "[DeleteRelaxedProblemExplorator] Total time for delete relaxed exploration: " << total_time << "\n"
              << "[DeleteRelaxedProblemExplorator] Number of fluent grounded atoms reachable in delete-free problem: "
              << m_delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count() << "\n"
              << "[DeleteRelaxedProblemExplorator] Number of derived grounded atoms reachable in delete-free problem: "
              << m_delete_free_state_repository.get_reached_derived_ground_atoms_bitset().count() << std::endl;
}

std::shared_ptr<GroundedAxiomEvaluator>
DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator(const match_tree::Options& options,
                                                                std::shared_ptr<IGroundedAxiomEvaluatorEventHandler> event_handler) const
{
    if (!event_handler)
    {
        event_handler = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>();
    }

    event_handler->on_start_ground_axiom_instantiation();
    auto start_time = std::chrono::high_resolution_clock::now();

    /* Initialize bookkeeping to map ground axioms into corresponding partition. */
    const auto num_partitions = m_problem->get_problem_and_domain_axiom_partitioning().size();
    auto axiom_to_partition = std::unordered_map<Axiom, size_t> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        const auto& partition = m_problem->get_problem_and_domain_axiom_partitioning()[i];
        for (const auto& axiom : partition.get_axioms())
        {
            axiom_to_partition.emplace(axiom, i);
        }
    }

    /* Store ground axioms in corresponding partition. */
    auto ground_axiom_partitioning = std::vector<GroundAxiomList>(num_partitions);
    for (const auto& ground_axiom : m_pddl_repositories->get_ground_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        const auto& axiom = m_pddl_repositories->get_axiom(ground_axiom->get_axiom_index());
        for (const auto& unrelaxed_axiom : m_delete_relax_transformer.get_unrelaxed_axioms(axiom))
        {
            auto axiom_arguments = m_pddl_repositories->get_objects_from_indices(ground_axiom->get_object_indices());
            auto grounded_axiom = m_pddl_repositories->ground(unrelaxed_axiom, m_problem, std::move(axiom_arguments));
            if (is_statically_applicable(grounded_axiom->get_conjunctive_condition(), m_problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_axiom_partitioning.at(axiom_to_partition.at(unrelaxed_axiom)).push_back(grounded_axiom);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_axiom_instantiation(total_time);

    event_handler->on_start_build_axiom_match_trees();
    start_time = std::chrono::high_resolution_clock::now();

    /* Create a MatchTree for each partition. */
    auto match_tree_partitioning = std::vector<std::unique_ptr<match_tree::MatchTree<GroundAxiomImpl>>> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        event_handler->on_start_build_axiom_match_tree(i);

        const auto& ground_axioms = ground_axiom_partitioning.at(i);

        auto match_tree = match_tree::MatchTree<GroundAxiomImpl>::create(*m_pddl_repositories, ground_axioms, options);

        event_handler->on_finish_build_axiom_match_tree(*match_tree);

        match_tree_partitioning.push_back(std::move(match_tree));
    }

    end_time = std::chrono::high_resolution_clock::now();
    total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_build_axiom_match_trees(total_time);

    return std::make_shared<GroundedAxiomEvaluator>(m_problem, m_pddl_repositories, std::move(match_tree_partitioning), std::move(event_handler));
}

std::shared_ptr<GroundedApplicableActionGenerator>
DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator(const match_tree::Options& options,
                                                                            std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler> event_handler) const
{
    if (!event_handler)
    {
        event_handler = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>();
    }

    event_handler->on_start_ground_action_instantiation();
    const auto start_time = std::chrono::high_resolution_clock::now();

    auto per_action_conditional_effects_candidate_objects = std::unordered_map<Action, std::vector<std::vector<IndexList>>> {};
    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        auto cond_effect_candidate_objects = std::vector<std::vector<IndexList>> {};
        cond_effect_candidate_objects.reserve(action->get_conditional_effects().size());

        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            auto [vertices_, vertices_by_parameter_index_, objects_by_parameter_index_] =
                consistency_graph::StaticConsistencyGraph::compute_vertices(m_problem,
                                                                            action->get_arity(),
                                                                            action->get_arity() + conditional_effect->get_arity(),
                                                                            conditional_effect->get_conjunctive_condition()->get_literals<Static>());

            cond_effect_candidate_objects.push_back(std::move(objects_by_parameter_index_));
        }

        per_action_conditional_effects_candidate_objects.emplace(action, std::move(cond_effect_candidate_objects));
    }

    auto ground_actions = GroundActionList {};
    for (const auto& action : m_pddl_repositories->get_ground_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_action : m_delete_relax_transformer.get_unrelaxed_actions(m_pddl_repositories->get_action(action->get_action_index())))
        {
            auto action_arguments = m_pddl_repositories->get_objects_from_indices(action->get_object_indices());
            const auto& conditional_effect_candidate_objects = per_action_conditional_effects_candidate_objects.at(unrelaxed_action);
            auto grounded_action = m_pddl_repositories->ground(unrelaxed_action, m_problem, std::move(action_arguments), conditional_effect_candidate_objects);
            if (is_statically_applicable(grounded_action->get_conjunctive_condition(), m_problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_actions.push_back(grounded_action);
            }
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_action_instantiation(total_time);

    event_handler->on_start_build_action_match_tree();

    auto match_tree = match_tree::MatchTree<GroundActionImpl>::create(*m_pddl_repositories, ground_actions, options);

    event_handler->on_finish_build_action_match_tree(*match_tree);

    return std::make_shared<GroundedApplicableActionGenerator>(m_problem, m_pddl_repositories, std::move(match_tree), std::move(event_handler));
}

Problem DeleteRelaxedProblemExplorator::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& DeleteRelaxedProblemExplorator::get_pddl_repositories() const { return m_pddl_repositories; }

}  // namespace mimir
