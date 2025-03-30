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
    m_action(action)
{
}

const Action& ActionSatisficingBindingGenerator::get_action() const { return m_action; }

bool ActionSatisficingBindingGenerator::is_valid_binding_impl(const DenseState& dense_state, const ObjectList& binding)
{
    return is_valid_binding(m_action->get_conjunctive_effect(), dense_state, binding)
           && std::all_of(m_action->get_conditional_effects().begin(),
                          m_action->get_conditional_effects().end(),
                          [&](auto&& arg) { return is_valid_binding_if_fires(arg, dense_state, binding); });
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
    const auto ground_function_expression = m_problem->ground(effect->get_function_expression(), binding);

    const auto result =
        (ground_target_function->get_index() < fluent_numeric_variables.size())
        && (fluent_numeric_variables[ground_target_function->get_index()] != UNDEFINED_CONTINUOUS_COST)
        && (evaluate(ground_function_expression, m_problem->get_initial_function_to_value<StaticTag>(), fluent_numeric_variables) != UNDEFINED_CONTINUOUS_COST);

    return result;
}

template<>
bool ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<AuxiliaryTag> effect,
                                                         const FlatDoubleList& fluent_numeric_variables,
                                                         const ObjectList& binding)
{
    // For auxiliary total-cost, we assume it is well-defined in the initial state.
    const auto ground_function_expression = m_problem->ground(effect->get_function_expression(), binding);

    return (evaluate(ground_function_expression, m_problem->get_initial_function_to_value<StaticTag>(), fluent_numeric_variables) != UNDEFINED_CONTINUOUS_COST);
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

bool ActionSatisficingBindingGenerator::is_valid_binding(ConjunctiveEffect effect, const DenseState& dense_state, const ObjectList& binding)
{
    return is_valid_binding(effect->get_fluent_numeric_effects(), dense_state.get_numeric_variables(), binding)
           && (!effect->get_auxiliary_numeric_effect().has_value()
               || is_valid_binding(effect->get_auxiliary_numeric_effect().value(), dense_state.get_numeric_variables(), binding));
}

bool ActionSatisficingBindingGenerator::is_valid_binding_if_fires(ConditionalEffect effect, const DenseState& dense_state, const ObjectList& binding)
{
    // Same idea as in is_applicable_if_fires.
    return !(!is_valid_binding(effect->get_conjunctive_effect(), dense_state, binding)  //
             && SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::is_valid_binding(effect->get_conjunctive_condition(), dense_state, binding));
}

template class SatisficingBindingGenerator<ActionSatisficingBindingGenerator>;

}
