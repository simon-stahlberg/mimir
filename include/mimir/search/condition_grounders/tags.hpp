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

#ifndef MIMIR_SEARCH_CONDITION_GROUNDER_TAGS_HPP_
#define MIMIR_SEARCH_CONDITION_GROUNDER_TAGS_HPP_

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
struct ConditionGrounderDispatcher
{
};

template<typename G>
struct is_condition_grounder_dispatcher : std::false_type
{
};

template<IsStateTag S>
struct is_condition_grounder_dispatcher<ConditionGrounderDispatcher<S>> : std::true_type
{
};

template<typename G>
concept IsConditionGrounder = is_condition_grounder_dispatcher<G>::value;

}

#endif