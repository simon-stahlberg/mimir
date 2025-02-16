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

ActionSatisficingBindingGenerator::ActionSatisficingBindingGenerator(Action action,
                                                                     Problem problem,
                                                                     std::shared_ptr<PDDLRepositories> pddl_repositories,
                                                                     std::optional<std::shared_ptr<ISatisficingBindingGeneratorEventHandler>> event_handler) :
    SatisficingBindingGenerator<ActionSatisficingBindingGenerator>(action->get_conjunctive_condition(), problem, pddl_repositories, event_handler),
    m_action(action)
{
}

bool ActionSatisficingBindingGenerator::is_valid_binding_impl(const DenseState& dense_state, const ObjectList& binding)
{
    return is_valid_binding(m_action->get_conjunctive_effect(), dense_state, binding)
           && std::all_of(m_action->get_conditional_effects().begin(),
                          m_action->get_conditional_effects().end(),
                          [&](auto&& arg) { return is_valid_binding_if_fires(arg, dense_state, binding); });
}

template<FluentOrAuxiliary F>
bool ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<F> effect, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding)
{
    return (evaluate(this->m_pddl_repositories->ground(effect->get_function_expression(), this->m_problem, binding), fluent_numeric_variables)
            != UNDEFINED_CONTINUOUS_COST);
}

template bool
ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<Fluent> effect, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding);
template bool
ActionSatisficingBindingGenerator::is_valid_binding(NumericEffect<Auxiliary> effect, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding);

template<FluentOrAuxiliary F>
bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<F>& effects,
                                                         const FlatDoubleList& fluent_numeric_variables,
                                                         const ObjectList& binding)
{
    for (const auto& effect : effects)
    {
        if (is_valid_binding(effect, fluent_numeric_variables, binding) == UNDEFINED_CONTINUOUS_COST)
        {
            return false;
        }
    }
    return true;
}

template bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<Fluent>& effects,
                                                                  const FlatDoubleList& fluent_numeric_variables,
                                                                  const ObjectList& binding);
template bool ActionSatisficingBindingGenerator::is_valid_binding(const NumericEffectList<Auxiliary>& effects,
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

}
