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

#include "mimir/formalism/action.hpp"

#include "mimir/formalism/effects.hpp"
#include "mimir/search/satisficing_binding_generators/action.hpp"
#include "mimir/search/satisficing_binding_generators/base_impl.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

ActionSatisficingBindingGenerator::ActionSatisficingBindingGenerator(Action action, Problem problem, std::shared_ptr<IEventHandler> event_handler) :
    SatisficingBindingGenerator<ActionSatisficingBindingGenerator>(action->get_conjunctive_condition(), problem, event_handler),
    m_action(action),
    m_fluent_numeric_changes(),
    m_auxiliary_numeric_change(detail::EffectFamily::NONE)
{
}

const Action& ActionSatisficingBindingGenerator::get_action() const { return m_action; }

bool ActionSatisficingBindingGenerator::is_valid_binding_impl(const UnpackedStateImpl& unpacked_state, const ObjectList& binding)
{
    // Reset
    m_fluent_numeric_changes.assign(unpacked_state.get_numeric_variables().size(), detail::EffectFamily::NONE);
    m_auxiliary_numeric_change = detail::EffectFamily::NONE;

    return std::all_of(m_action->get_conditional_effects().begin(),
                       m_action->get_conditional_effects().end(),
                       [&](auto&& arg) { return is_valid_binding_if_fires(arg, unpacked_state, binding); });
}

template<IsFluentOrAuxiliaryTag F>
bool ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<F> effect, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding)
{
    throw std::logic_error("Should not be called.");
}

template<>
bool ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<FluentTag> effect,
                                                         const FlatDoubleList& fluent_numeric_variables,
                                                         const ObjectList& binding)
{
    // For fluent, we have to check that the target function is well-defined.
    const auto ground_target_function = m_problem->ground(effect->get_function(), binding);

    const auto effect_index = ground_target_function->get_index();

    m_fluent_numeric_changes.resize(effect_index + 1, detail::EffectFamily::NONE);
    auto& recorded_effect_family = m_fluent_numeric_changes.at(effect_index);
    const auto effect_family = detail::get_effect_family(effect->get_assign_operator());

    const bool is_incompatible_change = (!detail::is_compatible_effect_family(recorded_effect_family, effect_family));

    if (is_incompatible_change)
        return false;

    recorded_effect_family = effect_family;

    const auto ground_function_expression = m_problem->ground(effect->get_function_expression(), binding);

    const auto is_assignment_operator = (effect->get_assign_operator() == loki::AssignOperatorEnum::ASSIGN);
    const auto is_undefined_value = (effect_index >= fluent_numeric_variables.size() || std::isnan(fluent_numeric_variables[effect_index]));

    if (is_undefined_value && !is_assignment_operator)
        return false;

    return !std::isnan(evaluate(ground_function_expression, m_problem->get_initial_function_to_value<StaticTag>(), fluent_numeric_variables));
}

template<>
bool ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<AuxiliaryTag> effect,
                                                         const FlatDoubleList& fluent_numeric_variables,
                                                         const ObjectList& binding)
{
    auto& recorded_effect_family = m_auxiliary_numeric_change;
    const auto effect_family = detail::get_effect_family(effect->get_assign_operator());

    const bool is_incompatible_change = (!detail::is_compatible_effect_family(recorded_effect_family, effect_family));

    if (is_incompatible_change)
        return false;

    recorded_effect_family = effect_family;

    // For auxiliary total-cost, we assume it is well-defined in the initial state.
    const auto ground_function_expression = m_problem->ground(effect->get_function_expression(), binding);

    return !std::isnan(evaluate(ground_function_expression, m_problem->get_initial_function_to_value<StaticTag>(), fluent_numeric_variables));
}

template<IsFluentOrAuxiliaryTag F>
bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<F>& effects,
                                                         const FlatDoubleList& fluent_numeric_variables,
                                                         const ObjectList& binding)
{
    for (const auto& effect : effects)
    {
        if (!is_valid_binding(effect, fluent_numeric_variables, binding))
        {
            return false;
        }
    }
    return true;
}

template bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<FluentTag>& effects,
                                                                  const FlatDoubleList& fluent_numeric_variables,
                                                                  const ObjectList& binding);
template bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<AuxiliaryTag>& effects,
                                                                  const FlatDoubleList& fluent_numeric_variables,
                                                                  const ObjectList& binding);

bool ActionSatisficingBindingGenerator::is_valid_binding(ConjunctiveEffect effect, const UnpackedStateImpl& unpacked_state, const ObjectList& binding)
{
    return is_valid_binding(effect->get_fluent_numeric_effects(), unpacked_state.get_numeric_variables(), binding)
           && (!effect->get_auxiliary_numeric_effect().has_value()
               || is_valid_binding(effect->get_auxiliary_numeric_effect().value(), unpacked_state.get_numeric_variables(), binding));
}

bool ActionSatisficingBindingGenerator::is_valid_binding_if_fires(ConditionalEffect effect, const UnpackedStateImpl& unpacked_state, const ObjectList& binding)
{
    // Same idea as in is_applicable_if_fires.
    return !(!is_valid_binding(effect->get_conjunctive_effect(), unpacked_state, binding)  //
             && SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::is_valid_binding(effect->get_conjunctive_condition(), unpacked_state, binding));
}

template class SatisficingBindingGenerator<ActionSatisficingBindingGenerator>;

}