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

#ifndef MIMIR_SEARCH_APPLICABILITY_HPP_
#define MIMIR_SEARCH_APPLICABILITY_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/**
 * ConjunctiveCondition
 */

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
extern bool nullary_conditions_hold(ConjunctiveCondition precondition, const DenseState& dense_state);

/**
 * GroundConjunctiveCondition
 */

extern bool is_dynamically_applicable(const GroundConjunctiveCondition& conjunctive_condition, const DenseState& dense_state);

extern bool is_statically_applicable(const GroundConjunctiveCondition& conjunctive_condition, const FlatBitset& static_positive_atoms);

extern bool is_applicable(const GroundConjunctiveCondition& conjunctive_condition, Problem problem, const DenseState& dense_state);

/**
 * GroundConjunctiveEffect
 */

extern bool is_dynamically_applicable(const GroundConjunctiveEffect& conjunctive_effect, const DenseState& dense_state);

extern bool is_statically_applicable(const GroundConjunctiveEffect& conjunctive_effect, Problem problem);

extern bool is_applicable(const GroundConjunctiveEffect& conjunctive_effect, Problem problem, const DenseState& dense_state);

/**
 * GroundConditionalEffect
 */

extern bool is_dynamically_applicable(const GroundConditionalEffect& conditional_effect, const DenseState& dense_state);

extern bool is_statically_applicable(const GroundConditionalEffect& conditional_effect, Problem problem);

extern bool is_applicable(const GroundConditionalEffect& conditional_effect, Problem problem, const DenseState& dense_state);

/**
 * GroundAction
 */

extern bool is_dynamically_applicable(GroundAction action, const DenseState& dense_state);

extern bool is_statically_applicable(GroundAction action, const FlatBitset& static_positive_atoms);

extern bool is_applicable(GroundAction action, Problem problem, const DenseState& dense_state);

/**
 * GroundAxiom
 */

extern bool is_dynamically_applicable(GroundAxiom axiom, const DenseState& dense_state);

extern bool is_statically_applicable(GroundAxiom axiom, const FlatBitset& static_positive_atoms);

extern bool is_applicable(GroundAxiom axiom, Problem problem, const DenseState& dense_state);
}

#endif
