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

#include "mimir/search/grounders/interface.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/grounded/grounded.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers/default.hpp"
#include "mimir/search/axiom_evaluators/grounded/grounded.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

IGrounder::IGrounder(Problem problem) : m_problem(std::move(problem)) {}

GroundedAxiomEvaluator IGrounder::create_grounded_axiom_evaluator(const match_tree::Options& options,
                                                                  GroundedAxiomEvaluatorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_axiom_instantiation();
    auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& repositories = problem.get_repositories();

    /* Initialize bookkeeping to map ground axioms into corresponding partition. */
    const auto num_partitions = problem.get_problem_and_domain_axiom_partitioning().size();
    auto axiom_to_partition = std::unordered_map<Axiom, size_t> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        const auto& partition = problem.get_problem_and_domain_axiom_partitioning()[i];
        for (const auto& axiom : partition.get_axioms())
        {
            axiom_to_partition.emplace(axiom, i);
        }
    }

    /* Store ground axioms in corresponding partition. */
    auto ground_axiom_partitioning = std::vector<GroundAxiomList>(num_partitions);
    for (const auto& ground_axiom : create_ground_axioms())
    {
        ground_axiom_partitioning.at(axiom_to_partition.at(ground_axiom->get_axiom())).push_back(ground_axiom);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_axiom_instantiation(total_time);

    event_handler->on_start_build_axiom_match_trees();
    start_time = std::chrono::high_resolution_clock::now();

    /* Create a MatchTree for each partition. */
    auto match_tree_partitioning = std::vector<std::unique_ptr<match_tree::MatchTreeImpl<GroundAxiomImpl>>> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        event_handler->on_start_build_axiom_match_tree(i);

        const auto& ground_axioms = ground_axiom_partitioning.at(i);

        auto match_tree = match_tree::MatchTreeImpl<GroundAxiomImpl>::create(repositories, ground_axioms, options);

        event_handler->on_finish_build_axiom_match_tree(*match_tree);

        match_tree_partitioning.push_back(std::move(match_tree));
    }

    end_time = std::chrono::high_resolution_clock::now();
    total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_build_axiom_match_trees(total_time);

    return GroundedAxiomEvaluatorImpl::create(m_problem, std::move(match_tree_partitioning), std::move(event_handler));
}

GroundedApplicableActionGenerator
IGrounder::create_grounded_applicable_action_generator(const match_tree::Options& options,
                                                       GroundedApplicableActionGeneratorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_action_instantiation();
    const auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& repositories = problem.get_repositories();

    auto ground_actions = create_ground_actions();

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_action_instantiation(total_time);

    event_handler->on_start_build_action_match_tree();

    auto match_tree = match_tree::MatchTreeImpl<GroundActionImpl>::create(repositories, ground_actions, options);

    event_handler->on_finish_build_action_match_tree(*match_tree);

    return GroundedApplicableActionGeneratorImpl::create(m_problem, std::move(match_tree), std::move(event_handler));
}

const Problem& IGrounder::get_problem() const { return m_problem; }

}
