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

#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/algorithms/siw/event_handlers.hpp"
#include "mimir/search/algorithms/siw/goal_strategy.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/grounding/action_grounder.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state_repository.hpp"

#include <sstream>

namespace mimir
{

/* ProblemGoalCounter */

ProblemGoalCounter::ProblemGoalCounter(Problem problem, State state) : m_problem(problem), m_initial_num_unsatisfied_goals(count_unsatisfied_goals(state)) {}

int ProblemGoalCounter::count_unsatisfied_goals(State state) const
{
    int num_unsatisfied_goals = 0;
    for (const auto& literal : m_problem->get_goal_condition<Fluent>())
    {
        if (!state->literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<Derived>())
    {
        if (!state->literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    return num_unsatisfied_goals;
}

bool ProblemGoalCounter::test_static_goal() { return m_problem->static_goal_holds(); }

bool ProblemGoalCounter::test_dynamic_goal(State state) { return count_unsatisfied_goals(state) < m_initial_num_unsatisfied_goals; }

/* SIW */

SerializedIterativeWidthAlgorithm::SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                                                     std::shared_ptr<StateRepository> state_repository,
                                                                     size_t max_arity) :
    SerializedIterativeWidthAlgorithm(std::move(applicable_action_generator),
                                      std::move(state_repository),
                                      max_arity,
                                      std::make_shared<DefaultBrFSAlgorithmEventHandler>(),
                                      std::make_shared<DefaultIWAlgorithmEventHandler>(),
                                      std::make_shared<DefaultSIWAlgorithmEventHandler>())
{
}

SerializedIterativeWidthAlgorithm::SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                                                     std::shared_ptr<StateRepository> state_repository,
                                                                     size_t max_arity,
                                                                     std::shared_ptr<IBrFSAlgorithmEventHandler> brfs_event_handler,
                                                                     std::shared_ptr<IIWAlgorithmEventHandler> iw_event_handler,
                                                                     std::shared_ptr<ISIWAlgorithmEventHandler> siw_event_handler) :
    m_applicable_action_generator(applicable_action_generator),
    m_state_repository(state_repository),
    m_max_arity(max_arity),
    m_brfs_event_handler(brfs_event_handler),
    m_iw_event_handler(iw_event_handler),
    m_siw_event_handler(siw_event_handler),
    m_initial_state(m_state_repository->get_or_create_initial_state()),
    m_iw(applicable_action_generator, state_repository, max_arity, brfs_event_handler, iw_event_handler)
{
    if (max_arity >= MAX_ARITY)
    {
        throw std::runtime_error("SerializedIterativeWidthAlgorithm::SerializedIterativeWidthAlgorithm(...): max_arity (" + std::to_string(max_arity)
                                 + ") cannot be greater than or equal to MAX_ARITY (" + std::to_string(MAX_ARITY) + ") compile time constant.");
    }
}

SearchResult SerializedIterativeWidthAlgorithm::find_solution() { return find_solution(m_initial_state); }

SearchResult SerializedIterativeWidthAlgorithm::find_solution(State start_state)
{
    auto result = SearchResult();

    const auto problem = m_applicable_action_generator->get_action_grounder().get_problem();
    const auto& pddl_repositories = *m_applicable_action_generator->get_action_grounder().get_pddl_repositories();
    m_siw_event_handler->on_start_search(problem, start_state, pddl_repositories);

    auto problem_goal_test = std::make_unique<ProblemGoal>(problem);

    if (!problem_goal_test->test_static_goal())
    {
        result.status = SearchStatus::UNSOLVABLE;
        return result;
    }

    auto cur_state = start_state;
    auto out_plan_actions = GroundActionList {};
    auto out_plan_cost = ContinuousCost(0);

    while (!problem_goal_test->test_dynamic_goal(cur_state))
    {
        // Run IW to decrease goal counter
        m_siw_event_handler->on_start_subproblem_search(problem, cur_state, pddl_repositories);

        auto partial_plan = std::optional<Plan> {};

        const auto sub_result = m_iw.find_solution(cur_state, std::make_unique<ProblemGoalCounter>(problem, cur_state));

        if (sub_result.status == SearchStatus::UNSOLVABLE)
        {
            m_siw_event_handler->on_end_search();
            m_siw_event_handler->on_unsolvable();

            result.status = SearchStatus::UNSOLVABLE;
            return result;
        }

        if (sub_result.status == SearchStatus::FAILED)
        {
            m_siw_event_handler->on_end_search();
            m_siw_event_handler->on_exhausted();

            result.status = SearchStatus::FAILED;
            return result;
        }

        cur_state = sub_result.goal_state.value();
        out_plan_actions.insert(out_plan_actions.end(), sub_result.plan.value().get_actions().begin(), sub_result.plan.value().get_actions().end());
        out_plan_cost += sub_result.plan.value().get_cost();

        m_siw_event_handler->on_end_subproblem_search(m_iw_event_handler->get_statistics());
    }

    m_siw_event_handler->on_end_search();
    if (!m_siw_event_handler->is_quiet())
    {
        m_applicable_action_generator->on_end_search();
        m_state_repository->get_axiom_evaluator()->on_end_search();
    }
    result.plan = Plan(std::move(out_plan_actions), out_plan_cost);
    m_siw_event_handler->on_solved(result.plan.value(), pddl_repositories);
    result.status = SearchStatus::SOLVED;
    return result;
}

}
