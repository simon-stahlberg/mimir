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

/// @brief Return true iff all nullary literals in the precondition hold, false otherwise.
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

/// @brief Return true iff all functions in numeric effects are well-defined in the state.
extern bool is_applicable(const GroundConjunctiveEffect& conjunctive_effect, const DenseState& dense_state);

/**
 * GroundConditionalEffect
 */

/// @brief Return true iff the conditional effect is applicable in the problem and state,
/// i.e., the conjunctive condition and the conjunctive effect are applicable.
/// More formally, consider conditional effect c = <pre, eff>.
/// app(c) <=> app(pre) && app(eff)
extern bool is_applicable(const GroundConditionalEffect& conditional_effect, Problem problem, const DenseState& dense_state);

/// @brief Return true iff the conditional effect is applicable in the problem and state when it fires,
/// i.e., when the conjunctive condition is applicable then the effect must also be applicable.
/// More formally, consider conditional effect c = <pre, eff>.
/// app(c) <=> app(pre) -> app(eff)
///        <=> !app(pre) || app(eff)
///        <=> !(app(pre) && !app(eff))
///        <=> !(!app(eff) && app(pre))
/// Now we have a more efficient form that tests app(eff) before app(pre).
extern bool is_applicable_if_fires(const GroundConditionalEffect& conditional_effect, Problem problem, const DenseState& dense_state);

/**
 * GroundAction
 */

/// @brief Return true iff the ground action is applicable in the problem and state.
extern bool is_applicable(GroundAction action, Problem problem, const DenseState& dense_state);

/**
 * GroundAxiom
 */

/// @brief Return true iff the ground axiom is applicable in the problem and state.
extern bool is_applicable(GroundAxiom axiom, Problem problem, const DenseState& dense_state);
}

#endif
