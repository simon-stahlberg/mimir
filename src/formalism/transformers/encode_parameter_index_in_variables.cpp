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

#include "mimir/formalism/transformers/encode_parameter_index_in_variables.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Variable EncodeParameterIndexInVariables::transform_impl(Variable variable)
{
    auto it = m_variable_to_parameter_index.find(variable);
    if (m_enable_encoding && it != m_variable_to_parameter_index.end())
    {
        const auto parameter_index = it->second;

        return m_pddl_repositories.get_or_create_variable(variable->get_name() + "_" + std::to_string(parameter_index), parameter_index);
    }
    return m_pddl_repositories.get_or_create_variable(variable->get_name(), 0);
}

template<StaticOrFluentOrDerived P>
Predicate<P> EncodeParameterIndexInVariables::transform_impl(Predicate<P> predicate)
{
    m_enable_encoding = false;

    const auto translated_predicate =
        this->m_pddl_repositories.template get_or_create_predicate<P>(predicate->get_name(), this->transform(predicate->get_parameters()));

    m_enable_encoding = true;

    return translated_predicate;
}

template Predicate<Static> EncodeParameterIndexInVariables::transform_impl(Predicate<Static> predicate);
template Predicate<Fluent> EncodeParameterIndexInVariables::transform_impl(Predicate<Fluent> predicate);
template Predicate<Derived> EncodeParameterIndexInVariables::transform_impl(Predicate<Derived> predicate);

template<StaticOrFluentOrAuxiliary F>
FunctionSkeleton<F> EncodeParameterIndexInVariables::transform_impl(FunctionSkeleton<F> function_skeleton)
{
    m_enable_encoding = false;

    const auto translated_function_skeleton =
        this->m_pddl_repositories.template get_or_create_function_skeleton<F>(function_skeleton->get_name(),
                                                                              this->transform(function_skeleton->get_parameters()));

    m_enable_encoding = true;

    return translated_function_skeleton;
}

template FunctionSkeleton<Static> EncodeParameterIndexInVariables::transform_impl(FunctionSkeleton<Static> function_skeleton);
template FunctionSkeleton<Fluent> EncodeParameterIndexInVariables::transform_impl(FunctionSkeleton<Fluent> function_skeleton);
template FunctionSkeleton<Auxiliary> EncodeParameterIndexInVariables::transform_impl(FunctionSkeleton<Auxiliary> function_skeleton);

ConditionalEffect EncodeParameterIndexInVariables::transform_impl(ConditionalEffect effect)
{
    // Determine variable parameter indices
    const auto start_index = m_variable_to_parameter_index.size();
    for (size_t i = 0; i < effect->get_arity(); ++i)
    {
        m_variable_to_parameter_index[effect->get_conjunctive_condition()->get_parameters()[i]] = start_index + i;
    }

    // Ensure in order translation
    const auto translated_conjunctive_condition = this->transform(effect->get_conjunctive_condition());
    const auto translated_conjunctive_effect = this->transform(effect->get_conjunctive_effect());
    const auto translated_conditional_effect =
        this->m_pddl_repositories.get_or_create_conditional_effect(translated_conjunctive_condition, translated_conjunctive_effect);

    // Erase for next universal effect
    for (size_t i = 0; i < effect->get_arity(); ++i)
    {
        m_variable_to_parameter_index.erase(effect->get_conjunctive_condition()->get_parameters()[i]);
    }

    return translated_conditional_effect;
}

Axiom EncodeParameterIndexInVariables::transform_impl(Axiom axiom)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < axiom->get_arity(); ++i)
    {
        m_variable_to_parameter_index[axiom->get_parameters()[i]] = i;
    }

    // Ensure in order translation
    const auto translated_conjunctive_condition = this->transform(axiom->get_conjunctive_condition());
    const auto translated_literal = this->transform(axiom->get_literal());
    const auto translated_axiom = this->m_pddl_repositories.get_or_create_axiom(translated_conjunctive_condition, translated_literal);

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_axiom;
}

Action EncodeParameterIndexInVariables::transform_impl(Action action)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < action->get_arity(); ++i)
    {
        m_variable_to_parameter_index[action->get_parameters()[i]] = i;
    }

    // Ensure in order translation
    const auto translated_precondition = this->transform(action->get_conjunctive_condition());
    const auto translated_conjunctive_effect = this->transform(action->get_conjunctive_effect());
    const auto translated_conditional_effects = this->transform(action->get_conditional_effects());
    const auto translated_action = this->m_pddl_repositories.get_or_create_action(action->get_name(),
                                                                                  action->get_original_arity(),
                                                                                  translated_precondition,
                                                                                  translated_conjunctive_effect,
                                                                                  translated_conditional_effects);

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_action;
}

Problem EncodeParameterIndexInVariables::run_impl(Problem problem) { return this->transform(problem); }

EncodeParameterIndexInVariables::EncodeParameterIndexInVariables(PDDLRepositories& pddl_repositories) :
    BaseRecurseTransformer<EncodeParameterIndexInVariables>(pddl_repositories)
{
}
}
