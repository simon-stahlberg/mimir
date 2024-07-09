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

Variable EncodeParameterIndexInVariables::transform_impl(const VariableImpl& variable)
{
    auto it = m_variable_to_parameter_index.find(&variable);
    if (m_enable_encoding && it != m_variable_to_parameter_index.end())
    {
        const auto parameter_index = it->second;

        return m_pddl_factories.get_or_create_variable(variable.get_name() + "_" + std::to_string(parameter_index), parameter_index);
    }
    return m_pddl_factories.get_or_create_variable(variable.get_name(), 0);
}

Predicate<Static> EncodeParameterIndexInVariables::transform_impl(const PredicateImpl<Static>& predicate)
{
    m_enable_encoding = false;

    const auto translated_predicate = this->m_pddl_factories.get_or_create_predicate<Static>(predicate.get_name(), this->transform(predicate.get_parameters()));

    m_enable_encoding = true;

    return translated_predicate;
}
Predicate<Fluent> EncodeParameterIndexInVariables::transform_impl(const PredicateImpl<Fluent>& predicate)
{
    m_enable_encoding = false;

    const auto translated_predicate = this->m_pddl_factories.get_or_create_predicate<Fluent>(predicate.get_name(), this->transform(predicate.get_parameters()));

    m_enable_encoding = true;

    return translated_predicate;
}
Predicate<Derived> EncodeParameterIndexInVariables::transform_impl(const PredicateImpl<Derived>& predicate)
{
    m_enable_encoding = false;

    const auto translated_predicate =
        this->m_pddl_factories.get_or_create_predicate<Derived>(predicate.get_name(), this->transform(predicate.get_parameters()));

    m_enable_encoding = true;

    return translated_predicate;
}

FunctionSkeleton EncodeParameterIndexInVariables::transform_impl(const FunctionSkeletonImpl& function_skeleton)
{
    m_enable_encoding = false;

    const auto translated_function_skeleton =
        this->m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(), this->transform(function_skeleton.get_parameters()));

    m_enable_encoding = true;

    return translated_function_skeleton;
}

EffectUniversal EncodeParameterIndexInVariables::transform_impl(const EffectUniversalImpl& effect)
{
    // Determine variable parameter indices
    const auto start_index = m_variable_to_parameter_index.size();
    for (size_t i = 0; i < effect.get_arity(); ++i)
    {
        m_variable_to_parameter_index[effect.get_parameters()[i]] = start_index + i;
    }

    const auto translated_universal_effect = this->m_pddl_factories.get_or_create_universal_effect(this->transform(effect.get_parameters()),
                                                                                                   this->transform(effect.get_conditions<Static>()),
                                                                                                   this->transform(effect.get_conditions<Fluent>()),
                                                                                                   this->transform(effect.get_conditions<Derived>()),
                                                                                                   this->transform(*effect.get_effect()));

    // Erase for next universal effect
    for (size_t i = 0; i < effect.get_arity(); ++i)
    {
        m_variable_to_parameter_index.erase(effect.get_parameters()[i]);
    }

    return translated_universal_effect;
}

Axiom EncodeParameterIndexInVariables::transform_impl(const AxiomImpl& axiom)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < axiom.get_arity(); ++i)
    {
        m_variable_to_parameter_index[axiom.get_parameters()[i]] = i;
    }

    const auto translated_parameters = this->transform(axiom.get_parameters());

    const auto translated_axiom = this->m_pddl_factories.get_or_create_axiom(translated_parameters,
                                                                             this->transform(*axiom.get_literal()),
                                                                             this->transform(axiom.get_conditions<Static>()),
                                                                             this->transform(axiom.get_conditions<Fluent>()),
                                                                             this->transform(axiom.get_conditions<Derived>()));

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_axiom;
}

Action EncodeParameterIndexInVariables::transform_impl(const ActionImpl& action)
{
    m_variable_to_parameter_index.clear();

    // Determine variable parameter indices
    for (size_t i = 0; i < action.get_arity(); ++i)
    {
        m_variable_to_parameter_index[action.get_parameters()[i]] = i;
    }

    const auto translated_parameters = this->transform(action.get_parameters());

    const auto translated_universal_effects = this->transform(action.get_universal_effects());

    const auto translated_action = this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                                               action.get_original_arity(),
                                                                               translated_parameters,
                                                                               this->transform(action.get_conditions<Static>()),
                                                                               this->transform(action.get_conditions<Fluent>()),
                                                                               this->transform(action.get_conditions<Derived>()),
                                                                               this->transform(action.get_simple_effects()),
                                                                               this->transform(action.get_conditional_effects()),
                                                                               translated_universal_effects,
                                                                               this->transform(*action.get_function_expression()));

    // Ensure that other translations definitely not use parameter indices
    m_variable_to_parameter_index.clear();

    return translated_action;
}

Problem EncodeParameterIndexInVariables::run_impl(const ProblemImpl& problem) { return this->transform(problem); }

EncodeParameterIndexInVariables::EncodeParameterIndexInVariables(PDDLFactories& pddl_factories) :
    BaseTransformer<EncodeParameterIndexInVariables>(pddl_factories)
{
}
}
