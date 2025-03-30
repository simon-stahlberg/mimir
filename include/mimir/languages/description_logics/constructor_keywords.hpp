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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_KEYWORDS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_KEYWORDS_HPP_

namespace mimir::languages::dl::keywords
{
// Concept keywords
inline constexpr auto concept_bot = "concept_bot";
inline constexpr auto concept_top = "concept_top";
inline constexpr auto concept_atomic_state = "concept_atomic_state";
inline constexpr auto concept_atomic_goal = "concept_atomic_goal";
inline constexpr auto concept_intersection = "concept_intersection";
inline constexpr auto concept_union = "concept_union";
inline constexpr auto concept_negation = "concept_negation";
inline constexpr auto concept_value_restriction = "concept_value_restriction";
inline constexpr auto concept_existential_quantification = "concept_existential_quantification";
inline constexpr auto concept_role_value_map_containment = "concept_role_value_map_containment";
inline constexpr auto concept_role_value_map_equality = "concept_role_value_map_equality";
inline constexpr auto concept_nominal = "concept_nominal";

// Role keywords
inline constexpr auto role_universal = "role_universal";
inline constexpr auto role_atomic_state = "role_atomic_state";
inline constexpr auto role_atomic_goal = "role_atomic_goal";
inline constexpr auto role_intersection = "role_intersection";
inline constexpr auto role_union = "role_union";
inline constexpr auto role_complement = "role_complement";
inline constexpr auto role_inverse = "role_inverse";
inline constexpr auto role_composition = "role_composition";
inline constexpr auto role_transitive_closure = "role_transitive_closure";
inline constexpr auto role_reflexive_transitive_closure = "role_reflexive_transitive_closure";
inline constexpr auto role_restriction = "role_restriction";
inline constexpr auto role_identity = "role_identity";

// Boolean keywords
inline constexpr auto boolean_atomic_state = "boolean_atomic_state";
inline constexpr auto boolean_nonempty = "boolean_nonempty";

// Numerical keywords
inline constexpr auto numerical_count = "numerical_count";
inline constexpr auto numerical_distance = "numerical_distance";
}

#endif
