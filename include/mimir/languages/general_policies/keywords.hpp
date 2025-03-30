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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_KEYWORDS_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_KEYWORDS_HPP_

namespace mimir::languages::general_policies::keywords
{

inline constexpr auto positive_boolean_condition = "positive_boolean_condition";
inline constexpr auto negative_boolean_condition = "negative_boolean_condition";
inline constexpr auto greater_numerical_condition = "greater_numerical_condition";
inline constexpr auto equal_numerical_condition = "equal_numerical_condition";

inline constexpr auto positive_boolean_effect = "positive_boolean_effect";
inline constexpr auto negative_boolean_effect = "negative_boolean_effect";
inline constexpr auto unchanged_boolean_effect = "unchanged_boolean_effect";
inline constexpr auto increase_numerical_effect = "increase_numerical_effect";
inline constexpr auto decrease_numerical_effect = "decrease_numerical_effect";
inline constexpr auto unchanged_numerical_effect = "unchanged_numerical_effect";

}

#endif
