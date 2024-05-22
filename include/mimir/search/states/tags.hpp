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

#ifndef MIMIR_SEARCH_STATES_TAGS_HPP_
#define MIMIR_SEARCH_STATES_TAGS_HPP_

#include <concepts>

namespace mimir
{

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct StateTag
{
};

template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsStateTag S>
struct StateDispatcher
{
};

template<typename T>
struct is_state_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_state_dispatcher<StateDispatcher<S>> : std::true_type
{
};

template<typename T>
concept IsStateDispatcher = is_state_dispatcher<T>::value;

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
class DenseStateTag : public StateTag
{
};

}

#endif
