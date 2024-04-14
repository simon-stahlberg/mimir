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

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

/**
 * Conditions
 */

loki::Condition flatten(const loki::ConditionAndImpl& condition, loki::PDDLFactories& pddl_factories)
{
    auto parts = loki::ConditionList {};
    for (const auto& part : condition.get_conditions())
    {
        if (const auto and_condition = std::get_if<loki::ConditionAndImpl>(part))
        {
            const auto nested_parts = std::get_if<loki::ConditionAndImpl>(flatten(*and_condition, pddl_factories));

            parts.insert(parts.end(), nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.push_back(part);
        }
    }
    return pddl_factories.get_or_create_condition_and(uniquify_elements(parts));
}

loki::Effect flatten(const loki::EffectAndImpl& effect, loki::PDDLFactories& pddl_factories)
{
    auto parts = loki::EffectList {};
    for (const auto& part : effect.get_effects())
    {
        if (const auto and_effect = std::get_if<loki::EffectAndImpl>(part))
        {
            const auto nested_parts = std::get_if<loki::EffectAndImpl>(flatten(*and_effect, pddl_factories));

            parts.insert(parts.end(), nested_parts->get_effects().begin(), nested_parts->get_effects().end());
        }
        else
        {
            parts.push_back(part);
        }
    }
    return pddl_factories.get_or_create_effect_and(uniquify_elements(parts));
}

loki::Condition flatten(const loki::ConditionOrImpl& condition, loki::PDDLFactories& pddl_factories)
{
    auto parts = loki::ConditionList {};
    for (const auto& part : condition.get_conditions())
    {
        if (const auto or_condition = std::get_if<loki::ConditionOrImpl>(part))
        {
            const auto nested_parts = std::get_if<loki::ConditionOrImpl>(flatten(*or_condition, pddl_factories));

            parts.insert(parts.end(), nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.push_back(part);
        }
    }
    return pddl_factories.get_or_create_condition_or(uniquify_elements(parts));
}

loki::Condition flatten(const loki::ConditionExistsImpl& condition, loki::PDDLFactories& pddl_factories)
{
    if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<loki::ConditionExistsImpl>(flatten(*condition_exists, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.get_or_create_condition_exists(parameters, nested_condition->get_condition());
    }
    return pddl_factories.get_or_create_condition_exists(condition.get_parameters(), condition.get_condition());
}

loki::Condition flatten(const loki::ConditionForallImpl& condition, loki::PDDLFactories& pddl_factories)
{
    if (const auto condition_forall = std::get_if<loki::ConditionForallImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<loki::ConditionForallImpl>(flatten(*condition_forall, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.get_or_create_condition_forall(parameters, nested_condition->get_condition());
    }
    return pddl_factories.get_or_create_condition_forall(condition.get_parameters(), condition.get_condition());
}

loki::Effect flatten(const loki::EffectConditionalWhenImpl& effect, loki::PDDLFactories& pddl_factories)
{
    if (const auto effect_when = std::get_if<loki::EffectConditionalWhenImpl>(effect.get_effect()))
    {
        const auto nested_effect = std::get_if<loki::EffectConditionalWhenImpl>(flatten(*effect_when, pddl_factories));

        return pddl_factories.get_or_create_effect_conditional_when(
            flatten(*std::get_if<loki::ConditionAndImpl>(pddl_factories.get_or_create_condition_and(
                        uniquify_elements(loki::ConditionList { effect.get_condition(), nested_effect->get_condition() }))),
                    pddl_factories),
            nested_effect->get_effect());
    }
    return pddl_factories.get_or_create_effect_conditional_when(effect.get_condition(), effect.get_effect());
}

void collect_free_variables_recursively(const loki::ConditionImpl& condition,
                                        std::unordered_set<loki::Variable>& ref_quantified_variables,
                                        std::unordered_set<loki::Variable>& ref_free_variables)
{
    if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(&condition))
    {
        for (const auto& term : condition_literal->get_literal()->get_atom()->get_terms())
        {
            if (const auto term_variable = std::get_if<loki::TermVariableImpl>(term))
            {
                if (!ref_free_variables.count(term_variable->get_variable()))
                {
                    ref_free_variables.insert(term_variable->get_variable());
                }
            }
        }
    }
    else if (const auto condition_imply = std::get_if<loki::ConditionImplyImpl>(&condition))
    {
        collect_free_variables_recursively(*condition_imply->get_condition_left(), ref_quantified_variables, ref_free_variables);
        collect_free_variables_recursively(*condition_imply->get_condition_right(), ref_quantified_variables, ref_free_variables);
    }
    else if (const auto condition_not = std::get_if<loki::ConditionNotImpl>(&condition))
    {
        collect_free_variables_recursively(*condition_not->get_condition(), ref_quantified_variables, ref_free_variables);
    }
    else if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(&condition))
    {
        for (const auto& part : condition_and->get_conditions())
        {
            collect_free_variables_recursively(*part, ref_quantified_variables, ref_free_variables);
        }
    }
    else if (const auto condition_or = std::get_if<loki::ConditionOrImpl>(&condition))
    {
        for (const auto& part : condition_or->get_conditions())
        {
            collect_free_variables_recursively(*part, ref_quantified_variables, ref_free_variables);
        }
    }
    else if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(&condition))
    {
        for (const auto& parameter : condition_exists->get_parameters())
        {
            ref_quantified_variables.insert(parameter->get_variable());
        }
        collect_free_variables_recursively(*condition_exists->get_condition(), ref_quantified_variables, ref_free_variables);
    }
    else if (const auto condition_forall = std::get_if<loki::ConditionForallImpl>(&condition))
    {
        for (const auto& parameter : condition_forall->get_parameters())
        {
            ref_quantified_variables.insert(parameter->get_variable());
        }
        collect_free_variables_recursively(*condition_forall->get_condition(), ref_quantified_variables, ref_free_variables);
    }
}

loki::VariableList collect_free_variables(const loki::ConditionImpl& condition)
{
    auto quantified_variables = std::unordered_set<loki::Variable> {};
    auto free_variables = std::unordered_set<loki::Variable> {};

    collect_free_variables_recursively(condition, quantified_variables, free_variables);

    return loki::VariableList(free_variables.begin(), free_variables.end());
}
}