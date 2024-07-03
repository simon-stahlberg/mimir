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

#ifndef MIMIR_DATASETS_TRANSITION_SYSTEM_HPP_
#define MIMIR_DATASETS_TRANSITION_SYSTEM_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/openlists.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <concepts>

namespace mimir
{

using TransitionCost = double;
using TransitionCostList = std::vector<TransitionCost>;
using TransitionIndex = int;
using TransitionIndexList = std::vector<TransitionIndex>;

template<typename T>
concept IsTransitionSystem = requires(T a, StateIndex state_index, TransitionIndex transition_index) {
    /* Memory */
    {
        a.get_pddl_parser()
    } -> std::convertible_to<const std::shared_ptr<PDDLParser>&>;
    {
        a.get_aag()
    } -> std::convertible_to<const std::shared_ptr<IAAG>&>;
    {
        a.get_ssg()
    } -> std::convertible_to<const std::shared_ptr<SuccessorStateGenerator>&>;

    /* States */
    {
        a.get_initial_state()
    } -> std::convertible_to<StateIndex>;
    {
        a.get_goal_states()
    } -> std::convertible_to<const StateIndexSet&>;
    {
        a.get_deadend_states()
    } -> std::convertible_to<const StateIndexSet&>;
    {
        a.get_forward_successor_adjacency_lists()
    } -> std::convertible_to<const std::vector<StateIndexList>&>;
    {
        a.get_backward_successor_adjacency_lists()
    } -> std::convertible_to<const std::vector<StateIndexList>&>;
    {
        a.get_num_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_goal_states()
    } -> std::convertible_to<size_t>;
    {
        a.get_num_deadend_states()
    } -> std::convertible_to<size_t>;
    {
        a.is_goal_state(state_index)
    } -> std::convertible_to<bool>;
    {
        a.is_deadend_state(state_index)
    } -> std::convertible_to<bool>;
    {
        a.is_alive_state(state_index)
    } -> std::convertible_to<bool>;

    /* Transitions */
    {
        a.get_num_transitions()
    } -> std::convertible_to<size_t>;
    {
        a.get_transition_cost(transition_index)
    } -> std::convertible_to<TransitionCost>;
    {
        a.get_forward_transition_adjacency_lists()
    } -> std::convertible_to<const std::vector<TransitionIndexList>&>;
    {
        a.get_backward_transition_adjacency_lists()
    } -> std::convertible_to<const std::vector<TransitionIndexList>&>;

    /* Distances */
    {
        a.get_goal_distances()
    } -> std::convertible_to<const std::vector<double>&>;
};

}

#endif
