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

#include "mimir/formalism/translator/encode_parameter_index_in_variables.hpp"

namespace mimir::formalism
{

Variable EncodeParameterIndexInVariables::translate_level_2(Variable variable, Repositories& repositories)
{
    auto it = m_variable_to_parameter_index.find(variable);
    if (m_enable_encoding && it != m_variable_to_parameter_index.end())
    {
        const auto parameter_index = it->second;

        return repositories.get_or_create_variable(variable->get_name() + "_" + std::to_string(parameter_index), parameter_index);
    }
    return repositories.get_or_create_variable(variable->get_name(), 0);
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> EncodeParameterIndexInVariables::translate_level_2(Predicate<P> predicate, Repositories& repositories)
{
    m_enable_encoding = false;

    const auto translated_predicate =
        repositories.template get_or_create_predicate<P>(predicate->get_name(), this->translate_level_0(predicate->get_parameters(), repositories));

    m_enable_encoding = true;

    return translated_predicate;
}

template Predicate<StaticTag> EncodeParameterIndexInVariables::translate_level_2(Predicate<StaticTag> predicate, Repositories& repositories);
template Predicate<FluentTag> EncodeParameterIndexInVariables::translate_level_2(Predicate<FluentTag> predicate, Repositories& repositories);
template Predicate<DerivedTag> EncodeParameterIndexInVariables::translate_level_2(Predicate<DerivedTag> predicate, Repositories& repositories);

template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeleton<F> EncodeParameterIndexInVariables::translate_level_2(FunctionSkeleton<F> function_skeleton, Repositories& repositories)
{
    m_enable_encoding = false;

    const auto translated_function_skeleton =
        repositories.template get_or_create_function_skeleton<F>(function_skeleton->get_name(),
                                                                 this->translate_level_0(function_skeleton->get_parameters(), repositories));

    m_enable_encoding = true;

    return translated_function_skeleton;
}

template FunctionSkeleton<StaticTag> EncodeParameterIndexInVariables::translate_level_2(FunctionSkeleton<StaticTag> function_skeleton,
                                                                                        Repositories& repositories);
template FunctionSkeleton<FluentTag> EncodeParameterIndexInVariables::translate_level_2(FunctionSkeleton<FluentTag> function_skeleton,
                                                                                        Repositories& repositories);
template FunctionSkeleton<AuxiliaryTag> EncodeParameterIndexInVariables::translate_level_2(FunctionSkeleton<AuxiliaryTag> function_skeleton,
                                                                                           Repositories& repositories);

ConditionalEffect EncodeParameterIndexInVariables::translate_level_2(ConditionalEffect effect, Repositories& repositories)
{
    // Determine variable parameter indices
    const auto start_index = m_variable_to_parameter_index.size();
    for (size_t i = 0; i < effect->get_arity(); ++i)
    {
        m_variable_to_parameter_index[effect->get_conjunctive_condition()->get_parameters()[i]] = start_index + i;
    }

    // Ensure in order translation
    const auto translated_conjunctive_condition = this->translate_level_0(effect->get_conjunctive_condition(), repositories);
    const auto translated_conjunctive_effect = this->translate_level_0(effect->get_conjunctive_effect(), repositories);
    const auto translated_conditional_effect = repositories.get_or_create_conditional_effect(translated_conjunctive_condition, translated_conjunctive_effect);

    // Erase for next universal effect
    for (size_t i = 0; i < effect->get_arity(); ++i)
    {
        m_variable_to_parameter_index.erase(effect->get_conjunctive_condition()->get_parameters()[i]);
    }

    return translated_conditional_effect;
}

Axiom EncodeParameterIndexInVariables::translate_level_2(Axiom axiom, Repositories& repositories)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < axiom->get_arity(); ++i)
    {
        m_variable_to_parameter_index[axiom->get_parameters()[i]] = i;
    }

    // Ensure in order translation
    const auto translated_conjunctive_condition = this->translate_level_0(axiom->get_conjunctive_condition(), repositories);
    const auto translated_literal = this->translate_level_0(axiom->get_literal(), repositories);
    const auto translated_axiom = repositories.get_or_create_axiom(translated_conjunctive_condition, translated_literal);

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_axiom;
}

Action EncodeParameterIndexInVariables::translate_level_2(Action action, Repositories& repositories)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < action->get_arity(); ++i)
    {
        m_variable_to_parameter_index[action->get_parameters()[i]] = i;
    }

    // Ensure in order translation
    const auto translated_precondition = this->translate_level_0(action->get_conjunctive_condition(), repositories);
    const auto translated_conjunctive_effect = this->translate_level_0(action->get_conjunctive_effect(), repositories);
    const auto translated_conditional_effects = this->translate_level_0(action->get_conditional_effects(), repositories);
    const auto translated_action = repositories.get_or_create_action(action->get_name(),
                                                                     action->get_original_arity(),
                                                                     translated_precondition,
                                                                     translated_conjunctive_effect,
                                                                     translated_conditional_effects);

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_action;
}

}
