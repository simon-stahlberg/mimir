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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TAGS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_TAGS_HPP_

#include "mimir/search/states/tags.hpp"

#include <concepts>

namespace mimir
{
/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct LiftedAAGDispatcher
{
};

template<IsStateTag S>
struct GroundedAAGDispatcher
{
};

template<typename T>
struct is_lifted_aag_dispatcher : std::false_type
{
};

template<typename T>
struct is_grounded_aag_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_lifted_aag_dispatcher<LiftedAAGDispatcher<S>> : std::true_type
{
};

template<IsStateTag S>
struct is_grounded_aag_dispatcher<GroundedAAGDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsAAGDispatcher = is_lifted_aag_dispatcher<T>::value || is_grounded_aag_dispatcher<T>::value;

}

#endif
