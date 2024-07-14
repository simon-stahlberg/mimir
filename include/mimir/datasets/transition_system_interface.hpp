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

#ifndef MIMIR_DATASETS_TRANSITION_SYSTEM_INTERFACE_HPP_
#define MIMIR_DATASETS_TRANSITION_SYSTEM_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/types.hpp"
#include "mimir/datasets/iterators.hpp"
#include "mimir/datasets/transition_interface.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace mimir
{

template<typename T>
concept IsTransitionSystem = requires(T a, StateIndex state_index, TransitionIndex transition_index) {
    /* Types */
    typename T::TransitionType;
    requires IsTransition<typename T::TransitionType>;

    /* Meta data */
    {
        a.get_problem()
    } -> std::same_as<Problem>;

    /* Memory */
    {
        a.get_pddl_factories()
    } -> std::same_as<const std::shared_ptr<PDDLFactories>&>;
    {
        a.get_aag()
    } -> std::same_as<const std::shared_ptr<IAAG>&>;
    {
        a.get_ssg()
    } -> std::same_as<const std::shared_ptr<SuccessorStateGenerator>&>;

    /* States */
    {
        a.get_initial_state()
    } -> std::same_as<StateIndex>;
    {
        a.get_goal_states()
    } -> std::same_as<const StateIndexSet&>;
    {
        a.get_deadend_states()
    } -> std::same_as<const StateIndexSet&>;
    {
        a.get_target_states(state_index)
    } -> std::same_as<TargetStateIndexIterator<typename T::TransitionType>>;
    {
        a.get_source_states(state_index)
    } -> std::same_as<SourceStateIndexIterator<typename T::TransitionType>>;
    {
        a.get_num_states()
    } -> std::same_as<size_t>;
    {
        a.get_num_goal_states()
    } -> std::same_as<size_t>;
    {
        a.get_num_deadend_states()
    } -> std::same_as<size_t>;
    {
        a.is_goal_state(state_index)
    } -> std::same_as<bool>;
    {
        a.is_deadend_state(state_index)
    } -> std::same_as<bool>;
    {
        a.is_alive_state(state_index)
    } -> std::same_as<bool>;

    /* Transitions */
    {
        a.get_transitions()
    } -> std::convertible_to<const std::vector<typename T::TransitionType>&>;
    {
        a.get_forward_transition_indices(state_index)
    } -> std::same_as<ForwardTransitionIndexIterator<typename T::TransitionType>>;
    {
        a.get_backward_transition_indices(state_index)
    } -> std::same_as<BackwardTransitionIndexIterator<typename T::TransitionType>>;
    {
        a.get_forward_transitions(state_index)
    } -> std::same_as<ForwardTransitionIterator<typename T::TransitionType>>;
    {
        a.get_backward_transitions(state_index)
    } -> std::same_as<BackwardTransitionIterator<typename T::TransitionType>>;

    /* Distances */
    {
        a.get_goal_distances()
    } -> std::same_as<const std::vector<double>&>;
};

}

#endif
