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

namespace mimir
{

ActionSatisficingBindingGenerator::ActionSatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                                                     std::shared_ptr<NumericConstraintGrounder> numeric_constraint_grounder,
                                                                     Action action,
                                                                     std::optional<std::shared_ptr<ISatisficingBindingGeneratorEventHandler>> event_handler) :
    SatisficingBindingGenerator<ActionSatisficingBindingGenerator>(literal_grounder,
                                                                   numeric_constraint_grounder,
                                                                   action->get_conjunctive_condition(),
                                                                   event_handler),
    m_action(action)
{
}

bool ActionSatisficingBindingGenerator::is_valid_dynamic_binding_impl(const DenseState& dense_state, const ObjectList& binding)
{
    const auto& fluent_numeric_variables = dense_state.get_numeric_variables();

    return is_valid_dynamic_binding(m_action->get_conjunctive_effect()->template get_numeric_effects<Fluent>(), fluent_numeric_variables, binding)
           && is_valid_dynamic_binding(m_action->get_conjunctive_effect()->template get_numeric_effects<Auxiliary>(), fluent_numeric_variables, binding)
           && std::all_of(
               m_action->get_conditional_effects().begin(),
               m_action->get_conditional_effects().end(),
               [&](auto&& arg)
               {
                   return is_valid_dynamic_binding(arg->get_conjunctive_effect()->template get_numeric_effects<Fluent>(), fluent_numeric_variables, binding)
                          && is_valid_dynamic_binding(arg->get_conjunctive_effect()->template get_numeric_effects<Auxiliary>(),
                                                      fluent_numeric_variables,
                                                      binding);
               });
}

template<DynamicFunctionTag F>
bool ActionSatisficingBindingGenerator::is_valid_dynamic_binding(const NumericEffectList<F>& effects,
                                                                 const FlatDoubleList& fluent_numeric_variables,
                                                                 const ObjectList& binding)
{
    auto& function_expression_grounder = *this->m_numeric_constraint_grounder->get_fexpr_grounder();

    for (const auto& effect : effects)
    {
        if (evaluate(function_expression_grounder.ground(effect->get_function_expression(), binding), fluent_numeric_variables) == UNDEFINED_CONTINUOUS_COST)
        {
            return false;
        }
    }
    return true;
}

template bool ActionSatisficingBindingGenerator::is_valid_dynamic_binding(const NumericEffectList<Fluent>& effects,
                                                                          const FlatDoubleList& fluent_numeric_variables,
                                                                          const ObjectList& binding);
template bool ActionSatisficingBindingGenerator::is_valid_dynamic_binding(const NumericEffectList<Auxiliary>& effects,
                                                                          const FlatDoubleList& fluent_numeric_variables,
                                                                          const ObjectList& binding);

}
