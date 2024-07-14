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
#include "mimir/search/algorithms/siw/event_handlers.hpp"
#include "mimir/search/algorithms/siw/goal_strategy.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <sstream>

namespace mimir
{

/* ProblemGoalCounter */

ProblemGoalCounter::ProblemGoalCounter(Problem problem, State state) : m_problem(problem), m_initial_num_unsatisfied_goals(count_unsatisfied_goals(state)) {}

int ProblemGoalCounter::count_unsatisfied_goals(const State state) const
{
    int num_unsatisfied_goals = 0;
    for (const auto& literal : m_problem->get_goal_condition<Fluent>())
    {
        if (!state.literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    for (const auto& literal : m_problem->get_goal_condition<Derived>())
    {
        if (!state.literal_holds(literal))
        {
            ++num_unsatisfied_goals;
        }
    }
    return num_unsatisfied_goals;
}

bool ProblemGoalCounter::test_static_goal() { return m_problem->static_goal_holds(); }

bool ProblemGoalCounter::test_dynamic_goal(const State state) { return count_unsatisfied_goals(state) < m_initial_num_unsatisfied_goals; }

/* SIW */

SerializedIterativeWidthAlgorithm::SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator, int max_arity) :
    SerializedIterativeWidthAlgorithm(applicable_action_generator,
                                      max_arity,
                                      std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                                      std::make_shared<DefaultBrFSAlgorithmEventHandler>(),
                                      std::make_shared<DefaultIWAlgorithmEventHandler>(),
                                      std::make_shared<DefaultSIWAlgorithmEventHandler>())
{
}

SerializedIterativeWidthAlgorithm::SerializedIterativeWidthAlgorithm(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                                                     int max_arity,
                                                                     std::shared_ptr<SuccessorStateGenerator> successor_state_generator,
                                                                     std::shared_ptr<IBrFSAlgorithmEventHandler> brfs_event_handler,
                                                                     std::shared_ptr<IIWAlgorithmEventHandler> iw_event_handler,
                                                                     std::shared_ptr<ISIWAlgorithmEventHandler> siw_event_handler) :
    m_aag(applicable_action_generator),
    m_max_arity(max_arity),
    m_ssg(successor_state_generator),
    m_brfs_event_handler(brfs_event_handler),
    m_iw_event_handler(iw_event_handler),
    m_siw_event_handler(siw_event_handler),
    m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>()),
    m_initial_state(m_ssg->get_or_create_initial_state()),
    m_iw(applicable_action_generator, max_arity, successor_state_generator, brfs_event_handler, iw_event_handler)
{
    if (max_arity < 0)
    {
        throw std::runtime_error("Arity must be greater of equal than 0.");
    }
}

SearchStatus SerializedIterativeWidthAlgorithm::find_solution(GroundActionList& out_plan) { return find_solution(m_initial_state, out_plan); }

SearchStatus SerializedIterativeWidthAlgorithm::find_solution(State start_state, GroundActionList& out_plan)
{
    std::optional<State> unused_out_state = std::nullopt;
    return find_solution(start_state, out_plan, unused_out_state);
}

SearchStatus SerializedIterativeWidthAlgorithm::find_solution(State start_state, GroundActionList& out_plan, std::optional<State>& out_goal_state)
{
    const auto problem = m_aag->get_problem();
    const auto& pddl_factories = *m_aag->get_pddl_factories();
    m_siw_event_handler->on_start_search(m_aag->get_problem(), start_state, pddl_factories);

    auto problem_goal_test = std::make_unique<ProblemGoal>(problem);

    if (!problem_goal_test->test_static_goal())
    {
        return SearchStatus::UNSOLVABLE;
    }

    auto cur_state = start_state;
    std::optional<State> goal_state = std::nullopt;

    while (!problem_goal_test->test_dynamic_goal(cur_state))
    {
        // Run IW to decrease goal counter
        m_siw_event_handler->on_start_subproblem_search(problem, cur_state, pddl_factories);

        auto partial_plan = GroundActionList {};

        auto search_status = m_iw.find_solution(cur_state, std::make_unique<ProblemGoalCounter>(problem, cur_state), partial_plan, goal_state);

        if (search_status == SearchStatus::UNSOLVABLE)
        {
            m_siw_event_handler->on_end_search();
            m_siw_event_handler->on_unsolvable();
            return SearchStatus::UNSOLVABLE;
        }

        if (search_status == SearchStatus::FAILED)
        {
            m_siw_event_handler->on_end_search();
            m_siw_event_handler->on_exhausted();
            return SearchStatus::FAILED;
        }

        assert(goal_state.has_value());
        cur_state = goal_state.value();
        out_plan.insert(out_plan.end(), partial_plan.begin(), partial_plan.end());

        m_siw_event_handler->on_end_subproblem_search(m_iw_event_handler->get_statistics());
    }

    out_goal_state = goal_state;
    m_siw_event_handler->on_end_search();
    if (!m_siw_event_handler->is_quiet())
    {
        m_aag->on_end_search();
    }
    m_siw_event_handler->on_solved(out_plan);
    return SearchStatus::SOLVED;
}

}
