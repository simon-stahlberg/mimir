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
#include "mimir/search/openlists.hpp"
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
        a.get_forward_successors(state_index)
    } -> std::same_as<DestinationStateIterator<typename T::TransitionType>>;
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
        // Ensure get_somethings returns const std::vector<T>&
        a.get_transitions()
    } -> std::same_as<const std::vector<typename T::TransitionType>&>;

    //{
    //    a.get_transitions_begin_by_source()
    //} -> std::convertible_to<const BeginIndexList&>;
    //{
    //    a.get_forward_transitions(state_index)
    //} -> IsTransitionIndexIteratorPair;
    //{
    //    a.get_backward_transitions(state_index)
    //} -> IsTransitionIndexIteratorPair;

    /* Distances */
    {
        a.get_goal_distances()
    } -> std::convertible_to<const std::vector<double>&>;
};

}

#endif
