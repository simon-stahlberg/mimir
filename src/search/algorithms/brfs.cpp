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

#include "mimir/search/algorithms/brfs.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/interface.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/grounding/action_grounder.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state_repository.hpp"

#include <deque>

namespace mimir
{

/**
 * BrFS search node
 */

struct BrFSSearchNodeTag
{
};

using BrFSSearchNodeImpl = SearchNodeImpl<DiscreteCost>;
using BrFSSearchNode = BrFSSearchNodeImpl*;
using ConstBrFSSearchNode = const BrFSSearchNodeImpl*;

static void set_g_value(BrFSSearchNode node, DiscreteCost g_value) { return set_property<0>(node, g_value); }

static DiscreteCost get_g_value(ConstBrFSSearchNode node) { return get_property<0>(node); }

static BrFSSearchNode
get_or_create_search_node(size_t state_index, const BrFSSearchNodeImpl& default_node, cista::storage::Vector<BrFSSearchNodeImpl>& search_nodes)
{
    while (state_index >= search_nodes.size())
    {
        search_nodes.push_back(default_node);
    }
    return search_nodes[state_index];
}

/**
 * BrFS
 */

BrFSAlgorithm::BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, std::shared_ptr<StateRepository> state_repository) :
    BrFSAlgorithm(applicable_action_generator, std::move(state_repository), std::make_shared<DefaultBrFSAlgorithmEventHandler>())
{
}

BrFSAlgorithm::BrFSAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                             std::shared_ptr<StateRepository> state_repository,
                             std::shared_ptr<IBrFSAlgorithmEventHandler> event_handler) :
    m_applicable_action_generator(std::move(applicable_action_generator)),
    m_state_repository(std::move(state_repository)),
    m_event_handler(std::move(event_handler))
{
}

SearchResult BrFSAlgorithm::find_solution() { return find_solution(m_state_repository->get_or_create_initial_state()); }

SearchResult BrFSAlgorithm::find_solution(State start_state)
{
    return find_solution(start_state,
                         std::make_unique<ProblemGoal>(m_applicable_action_generator->get_action_grounder().get_problem()),
                         std::make_unique<DuplicateStatePruning>());
}

SearchResult
BrFSAlgorithm::find_solution(State start_state, std::unique_ptr<IGoalStrategy>&& goal_strategy, std::unique_ptr<IPruningStrategy>&& pruning_strategy)
{
    auto result = SearchResult();
    auto default_search_node =
        BrFSSearchNodeImpl { SearchNodeStatus::NEW, std::numeric_limits<Index>::max(), std::numeric_limits<Index>::max(), DiscreteCost(0) };
    auto search_nodes = cista::storage::Vector<BrFSSearchNodeImpl>();
    auto queue = std::deque<State>();

    const auto problem = m_applicable_action_generator->get_action_grounder().get_problem();
    const auto& pddl_repositories = *m_applicable_action_generator->get_action_grounder().get_pddl_repositories();
    m_event_handler->on_start_search(start_state, problem, pddl_repositories);

    auto start_search_node = get_or_create_search_node(start_state->get_index(), default_search_node, search_nodes);
    set_status(start_search_node, SearchNodeStatus::OPEN);
    set_g_value(start_search_node, 0);

    if (!goal_strategy->test_static_goal())
    {
        m_event_handler->on_unsolvable();

        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto applicable_actions = GroundActionList {};

    if (pruning_strategy->test_prune_initial_state(start_state))
    {
        result.status = SearchStatus::FAILED;
        return result;
    }

    queue.emplace_back(start_state);

    auto g_value = DiscreteCost(0);

    while (!queue.empty())
    {
        const auto state = queue.front();
        queue.pop_front();

        // We need this before goal test for correct statistics reporting.
        auto search_node = get_or_create_search_node(state->get_index(), default_search_node, search_nodes);

        if (get_g_value(search_node) > g_value)
        {
            g_value = get_g_value(search_node);
            m_applicable_action_generator->on_finish_search_layer();
            m_state_repository->get_axiom_evaluator()->on_finish_search_layer();
            m_event_handler->on_finish_g_layer();
        }

        if (goal_strategy->test_dynamic_goal(state))
        {
            auto plan_actions = GroundActionList {};
            set_plan(search_nodes, m_applicable_action_generator->get_action_grounder().get_ground_actions(), search_node, plan_actions);
            result.goal_state = state;
            result.plan = Plan(std::move(plan_actions), get_g_value(search_node));
            m_event_handler->on_end_search();
            if (!m_event_handler->is_quiet())
            {
                m_applicable_action_generator->on_end_search();
                m_state_repository->get_axiom_evaluator()->on_end_search();
            }
            m_event_handler->on_solved(result.plan.value(), pddl_repositories);

            result.status = SearchStatus::SOLVED;
            return result;
        }

        m_event_handler->on_expand_state(state, problem, pddl_repositories);

        for (const auto& action : this->m_applicable_action_generator->create_applicable_action_generator(state))
        {
            /* Open state. */
            const auto [successor_state, action_cost] = this->m_state_repository->get_or_create_successor_state(state, action);
            auto successor_search_node = get_or_create_search_node(successor_state->get_index(), default_search_node, search_nodes);

            m_event_handler->on_generate_state(successor_state, action, problem, pddl_repositories);

            const bool is_new_successor_state = (get_status(successor_search_node) == SearchNodeStatus::NEW);
            if (pruning_strategy->test_prune_successor_state(state, successor_state, is_new_successor_state))
            {
                m_event_handler->on_prune_state(successor_state, problem, pddl_repositories);
                continue;
            }

            set_status(successor_search_node, SearchNodeStatus::OPEN);
            set_parent_state(successor_search_node, state->get_index());
            set_creating_action(successor_search_node, action->get_index());
            set_g_value(successor_search_node, get_g_value(search_node) + 1);

            queue.emplace_back(successor_state);
        }

        /* Close state. */
        set_status(search_node, SearchNodeStatus::CLOSED);
    }

    m_event_handler->on_end_search();
    m_event_handler->on_exhausted();

    result.status = SearchStatus::EXHAUSTED;
    return result;
}

}
