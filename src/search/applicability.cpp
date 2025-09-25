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

#include "mimir/search/applicability.hpp"

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"
#include "mimir/formalism/ground_effects.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_unpacked.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

/**
 * Thread-local scratch-pad to check conflicting effect changes
 */
static thread_local std::vector<std::optional<loki::AssignOperatorEnum>> s_fluent_numeric_changes;
static thread_local std::optional<loki::AssignOperatorEnum> s_auxiliary_numeric_change;

/**
 * ConjunctiveCondition
 */

template<IsStaticOrFluentOrDerivedTag P>
static bool nullary_literals_hold(const GroundLiteralList<P>& literals, const FlatBitset& atom_indices)
{
    for (const auto& literal : literals)
    {
        if (literal->get_polarity() != atom_indices.get(literal->get_atom()->get_index()))
        {
            return false;
        }
    }

    return true;
}

static bool nullary_constraints_hold(const GroundNumericConstraintList& constraints, const UnpackedStateImpl& unpacked_state)
{
    for (const auto& constraint : constraints)
    {
        if (!evaluate(constraint, unpacked_state.get_problem().get_initial_function_to_value<StaticTag>(), unpacked_state.get_numeric_variables()))
        {
            return false;
        }
    }

    return true;
}

bool nullary_conditions_hold(ConjunctiveCondition conjunctive_condition, const UnpackedStateImpl& unpacked_state)
{
    // Note: checking nullary constraints doesnt work because its value is problem-dependent!
    return nullary_literals_hold(conjunctive_condition->get_nullary_ground_literals<FluentTag>(), unpacked_state.get_atoms<FluentTag>())
           && nullary_literals_hold(conjunctive_condition->get_nullary_ground_literals<DerivedTag>(), unpacked_state.get_atoms<DerivedTag>())
           && nullary_constraints_hold(conjunctive_condition->get_nullary_ground_numeric_constraints(), unpacked_state);
}

/**
 * GroundConjunctiveCondition
 */

template<IsStaticOrFluentOrDerivedTag P, std::ranges::input_range Range>
    requires IsRangeOver<Range, Index>
bool is_applicable(GroundConjunctiveCondition conjunctive_condition, const Range& atoms)
{
    return is_supseteq(atoms, conjunctive_condition->template get_precondition<PositiveTag, P>())  //
           && are_disjoint(atoms, conjunctive_condition->template get_precondition<NegativeTag, P>());
}

/// @brief Tests whether the fluents are statically applicable, i.e., contain no conflicting literals.
/// Without this test we can observe conflicting preconditions in the test_problem of the Ferry domain.
template<IsFluentOrDerivedTag P>
bool is_statically_applicable(GroundConjunctiveCondition conjunctive_condition)
{
    return are_disjoint(conjunctive_condition->template get_precondition<PositiveTag, P>(), conjunctive_condition->template get_precondition<NegativeTag, P>());
}

static bool
is_applicable(GroundConjunctiveCondition conjunctive_condition, const FlatDoubleList& static_numeric_variables, const FlatDoubleList& fluent_numeric_variables)
{
    for (const auto& constraint : conjunctive_condition->get_numeric_constraints())
    {
        if (!evaluate(constraint, static_numeric_variables, fluent_numeric_variables))
        {
            return false;
        }
    }
    return true;
}

bool is_dynamically_applicable(GroundConjunctiveCondition conjunctive_condition, const UnpackedStateImpl& unpacked_state)
{
    return is_applicable<FluentTag>(conjunctive_condition, unpacked_state.get_atoms<FluentTag>())
           && is_applicable<DerivedTag>(conjunctive_condition, unpacked_state.get_atoms<DerivedTag>())
           && is_applicable(conjunctive_condition,
                            unpacked_state.get_problem().get_initial_function_to_value<StaticTag>(),
                            unpacked_state.get_numeric_variables());
}

bool is_dynamically_applicable(GroundConjunctiveCondition conjunctive_condition, const State& state)
{
    return is_dynamically_applicable(conjunctive_condition, state.get_unpacked_state());
}

bool is_statically_applicable(GroundConjunctiveCondition conjunctive_condition, const FlatBitset& static_positive_atoms)
{
    return is_applicable<StaticTag>(conjunctive_condition, static_positive_atoms)  //
           && is_statically_applicable<FluentTag>(conjunctive_condition)           //
           && is_statically_applicable<DerivedTag>(conjunctive_condition);
}

bool is_applicable(GroundConjunctiveCondition conjunctive_condition, const UnpackedStateImpl& unpacked_state)
{
    return is_dynamically_applicable(conjunctive_condition, unpacked_state)
           && is_statically_applicable(conjunctive_condition, unpacked_state.get_problem().get_positive_static_initial_atoms_bitset());
}

bool is_applicable(GroundConjunctiveCondition conjunctive_condition, const State& state)
{
    return is_applicable(conjunctive_condition, state.get_unpacked_state());
}

/**
 * GroundConjunctiveEffect
 */

static bool is_applicable(GroundNumericEffect<FluentTag> effect,
                          const FlatDoubleList& static_numeric_variables,
                          const FlatDoubleList& fluent_numeric_variables,
                          std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes)
{
    const auto effect_index = effect->get_function()->get_index();

    s_fluent_numeric_changes.resize(effect_index + 1, std::nullopt);
    auto& recorded_change = s_fluent_numeric_changes.at(effect_index);
    const bool is_incompatible_change = (recorded_change && !is_compatible_numeric_effect(recorded_change.value(), effect->get_assign_operator()));

    if (is_incompatible_change)
        return false;
    recorded_change = effect->get_assign_operator();

    const auto is_update = (effect->get_assign_operator() != loki::AssignOperatorEnum::ASSIGN);
    const auto modifies_undefined = (effect_index >= fluent_numeric_variables.size() || std::isnan(fluent_numeric_variables[effect_index]));

    if (modifies_undefined && is_update)
    {
        return false;
    }

    return !std::isnan(evaluate(effect->get_function_expression(), static_numeric_variables, fluent_numeric_variables));
}

static bool is_applicable(GroundNumericEffect<AuxiliaryTag> effect,
                          const FlatDoubleList& static_numeric_variables,
                          const FlatDoubleList& fluent_numeric_variables,
                          std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    auto& recorded_change = s_auxiliary_numeric_change;
    const bool is_incompatible_change = (s_auxiliary_numeric_change && !is_compatible_numeric_effect(recorded_change.value(), effect->get_assign_operator()));

    if (is_incompatible_change)
        return false;
    recorded_change = effect->get_assign_operator();

    // For auxiliary total-cost, we assume it is well-defined in the initial state.
    return !std::isnan(evaluate(effect->get_function_expression(), static_numeric_variables, fluent_numeric_variables));
}

/// @brief Return true iff the fluent numeric effects are applicable, i.e., all numeric effects are well-defined.
static bool is_applicable(const GroundNumericEffectList<FluentTag>& effects,
                          const FlatDoubleList& static_numeric_variables,
                          const FlatDoubleList& fluent_numeric_variables,
                          std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes)
{
    for (const auto& effect : effects)
    {
        if (!is_applicable(effect, static_numeric_variables, fluent_numeric_variables, s_fluent_numeric_changes))
        {
            return false;
        }
    }
    return true;
}

bool is_applicable(GroundConjunctiveEffect conjunctive_effect,
                   const UnpackedStateImpl& unpacked_state,
                   std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                   std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    assert(s_fluent_numeric_changes.size() == unpacked_state.get_numeric_variables().size());

    return is_applicable(conjunctive_effect->get_fluent_numeric_effects(),
                         unpacked_state.get_problem().get_initial_function_to_value<StaticTag>(),
                         unpacked_state.get_numeric_variables(),
                         s_fluent_numeric_changes)
           && (!conjunctive_effect->get_auxiliary_numeric_effect().has_value()
               || is_applicable(conjunctive_effect->get_auxiliary_numeric_effect().value(),
                                unpacked_state.get_problem().get_initial_function_to_value<StaticTag>(),
                                unpacked_state.get_numeric_variables(),
                                s_auxiliary_numeric_change));
}

bool is_applicable(GroundConjunctiveEffect conjunctive_effect,
                   const State& state,
                   std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                   std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    return is_applicable(conjunctive_effect, state.get_unpacked_state(), s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable(GroundConjunctiveEffect conjunctive_effect, const UnpackedStateImpl& unpacked_state)
{
    s_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), std::nullopt);
    s_auxiliary_numeric_change = std::nullopt;

    return is_applicable(conjunctive_effect, unpacked_state, s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable(GroundConjunctiveEffect conjunctive_effect, const State& state) { return is_applicable(conjunctive_effect, state.get_unpacked_state()); }

/**
 * GroundConditionalEffect
 */

bool is_applicable(GroundConditionalEffect conditional_effect,
                   const UnpackedStateImpl& unpacked_state,
                   std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                   std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    assert(s_fluent_numeric_changes.size() == unpacked_state.get_numeric_variables().size());

    return is_applicable(conditional_effect->get_conjunctive_condition(), unpacked_state)  //
           && is_applicable(conditional_effect->get_conjunctive_effect(), unpacked_state, s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable(GroundConditionalEffect conditional_effect,
                   const State& state,
                   std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                   std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    return is_applicable(conditional_effect, state.get_unpacked_state(), s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable(GroundConditionalEffect conditional_effect, const UnpackedStateImpl& unpacked_state)
{
    s_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), std::nullopt);
    s_auxiliary_numeric_change = std::nullopt;

    return is_applicable(conditional_effect, unpacked_state, s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable(GroundConditionalEffect conditional_effect, const State& state) { return is_applicable(conditional_effect, state.get_unpacked_state()); }

bool is_applicable_if_fires(GroundConditionalEffect conditional_effect,
                            const UnpackedStateImpl& unpacked_state,
                            std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                            std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    assert(s_fluent_numeric_changes.size() == unpacked_state.get_numeric_variables().size());

    return !(!is_applicable(conditional_effect->get_conjunctive_effect(), unpacked_state, s_fluent_numeric_changes, s_auxiliary_numeric_change)  //
             && is_applicable(conditional_effect->get_conjunctive_condition(), unpacked_state));
}

bool is_applicable_if_fires(GroundConditionalEffect conditional_effect,
                            const State& state,
                            std::vector<std::optional<loki::AssignOperatorEnum>>& s_fluent_numeric_changes,
                            std::optional<loki::AssignOperatorEnum>& s_auxiliary_numeric_change)
{
    return is_applicable_if_fires(conditional_effect, state.get_unpacked_state(), s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable_if_fires(GroundConditionalEffect conditional_effect, const UnpackedStateImpl& unpacked_state)
{
    s_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), std::nullopt);
    s_auxiliary_numeric_change = std::nullopt;

    return is_applicable_if_fires(conditional_effect, unpacked_state, s_fluent_numeric_changes, s_auxiliary_numeric_change);
}

bool is_applicable_if_fires(GroundConditionalEffect conditional_effect, const State& state)
{
    return is_applicable_if_fires(conditional_effect, state.get_unpacked_state());
}

/**
 * GroundAction
 */

bool is_dynamically_applicable(GroundAction action, const UnpackedStateImpl& unpacked_state)
{
    s_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), std::nullopt);
    s_auxiliary_numeric_change = std::nullopt;

    return is_dynamically_applicable(action->get_conjunctive_condition(), unpacked_state)  //
           && std::all_of(action->get_conditional_effects().begin(),
                          action->get_conditional_effects().end(),
                          [&](auto&& arg) { return is_applicable_if_fires(arg, unpacked_state); });
}

bool is_applicable(GroundAction action, const UnpackedStateImpl& unpacked_state)
{
    s_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), std::nullopt);
    s_auxiliary_numeric_change = std::nullopt;

    return is_applicable(action->get_conjunctive_condition(), unpacked_state)  //
           && std::all_of(action->get_conditional_effects().begin(),
                          action->get_conditional_effects().end(),
                          [&](auto&& arg) { return is_applicable_if_fires(arg, unpacked_state); });
}

bool is_applicable(GroundAction action, const State& state) { return is_applicable(action, state.get_unpacked_state()); }

/**
 * GroundAxiom
 */

bool is_dynamically_applicable(GroundAxiom axiom, const UnpackedStateImpl& unpacked_state)
{
    return is_dynamically_applicable(axiom->get_conjunctive_condition(), unpacked_state);
}

bool is_applicable(GroundAxiom axiom, const UnpackedStateImpl& unpacked_state) { return is_applicable(axiom->get_conjunctive_condition(), unpacked_state); }
}
