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

#include "mimir/search/algorithms/siw.hpp"

#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/algorithms/siw/event_handlers.hpp"
#include "mimir/search/algorithms/siw/event_handlers/interface.hpp"
#include "mimir/search/algorithms/siw/goal_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <sstream>

using namespace mimir::formalism;

namespace mimir::search::siw
{

/* ProblemGoalStrategyImplCounter */

ProblemGoalStrategyImplCounter::ProblemGoalStrategyImplCounter(Problem problem, State state) :
    m_problem(problem),
    m_initial_num_unsatisfied_goals(count_unsatisfied_goals(state))
{
}

int ProblemGoalStrategyImplCounter::count_unsatisfied_goals(State state) const
{
    int num_unsatisfied_goals = 0;
    for (const auto& literal : m_problem->get_goal_condition<FluentTag>())
    {
        if (!state->literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<DerivedTag>())
    {
        if (!state->literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    return num_unsatisfied_goals;
}

bool ProblemGoalStrategyImplCounter::test_static_goal() { return m_problem->static_goal_holds(); }

bool ProblemGoalStrategyImplCounter::test_dynamic_goal(State state) { return count_unsatisfied_goals(state) < m_initial_num_unsatisfied_goals; }

/* SIW */
SearchResult find_solution(const SearchContext& context,
                           State start_state_,
                           size_t max_arity_,
                           EventHandler siw_event_handler_,
                           iw::EventHandler iw_event_handler_,
                           brfs::EventHandler brfs_event_handler_,
                           GoalStrategy goal_strategy_)
{
    auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto max_arity = max_arity_;
    const auto [start_state, start_g_value] =
        (start_state_) ? std::make_pair(start_state_, compute_state_metric_value(start_state_, problem)) : state_repository.get_or_create_initial_state();
    const auto siw_event_handler = (siw_event_handler_) ? siw_event_handler_ : DefaultEventHandlerImpl::create(context->get_problem());
    const auto iw_event_handler = (iw_event_handler_) ? iw_event_handler_ : iw::DefaultEventHandlerImpl::create(context->get_problem());
    const auto brfs_event_handler = (brfs_event_handler_) ? brfs_event_handler_ : brfs::DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (goal_strategy_) ? goal_strategy_ : ProblemGoalStrategyImpl::create(context->get_problem());

    if (max_arity >= iw::MAX_ARITY)
    {
        throw std::runtime_error("siw::find_solution(...): max_arity (" + std::to_string(max_arity) + ") cannot be greater than or equal to MAX_ARITY ("
                                 + std::to_string(iw::MAX_ARITY) + ") compile time constant.");
    }

    auto result = SearchResult();

    siw_event_handler->on_start_search(start_state);

    if (!goal_strategy->test_static_goal())
    {
        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto cur_state = start_state;
    auto out_plan_actions = GroundActionList {};
    auto out_plan_cost = ContinuousCost(0);

    while (!goal_strategy->test_dynamic_goal(cur_state))
    {
        // Run IW to decrease goal counter
        siw_event_handler->on_start_subproblem_search(cur_state);

        auto partial_plan = std::optional<Plan> {};

        const auto sub_result = iw::find_solution(context,
                                                  cur_state,
                                                  max_arity,
                                                  iw_event_handler,
                                                  brfs_event_handler,
                                                  std::make_shared<ProblemGoalStrategyImplCounter>(context->get_problem(), cur_state));

        if (sub_result.status == SearchStatus::UNSOLVABLE)
        {
            siw_event_handler->on_end_search();
            siw_event_handler->on_unsolvable();

            result.status = SearchStatus::UNSOLVABLE;
            return result;
        }

        if (sub_result.status == SearchStatus::FAILED)
        {
            siw_event_handler->on_end_search();
            siw_event_handler->on_exhausted();

            result.status = SearchStatus::FAILED;
            return result;
        }

        cur_state = sub_result.goal_state.value();
        out_plan_actions.insert(out_plan_actions.end(), sub_result.plan.value().get_actions().begin(), sub_result.plan.value().get_actions().end());
        out_plan_cost += sub_result.plan.value().get_cost();

        siw_event_handler->on_end_subproblem_search(iw_event_handler->get_statistics());
    }

    siw_event_handler->on_end_search();
    if (!siw_event_handler->is_quiet())
    {
        applicable_action_generator.on_end_search();
        state_repository.get_axiom_evaluator()->on_end_search();
    }
    result.plan = Plan(std::move(out_plan_actions), out_plan_cost);
    siw_event_handler->on_solved(result.plan.value());
    result.status = SearchStatus::SOLVED;
    return result;
}
}
