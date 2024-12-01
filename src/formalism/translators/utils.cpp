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

#include "mimir/common/collections.hpp"
#include "mimir/common/hash.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>

using namespace std::string_literals;

namespace mimir
{

/**
 * Conditions
 */

loki::Condition flatten(const loki::ConditionAndImpl& condition, loki::PDDLRepositories& pddl_repositories)
{
    auto parts = loki::ConditionList {};
    for (const auto& part : condition.get_conditions())
    {
        if (const auto and_condition = std::get_if<loki::ConditionAnd>(&part->get_condition()))
        {
            const auto& nested_parts = *std::get<loki::ConditionAnd>(flatten(**and_condition, pddl_repositories)->get_condition());

            parts.insert(parts.end(), nested_parts.get_conditions().begin(), nested_parts.get_conditions().end());
        }
        else
        {
            parts.push_back(part);
        }
    }
    return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_and(uniquify_elements(parts)));
}

loki::Condition flatten(const loki::ConditionOrImpl& condition, loki::PDDLRepositories& pddl_repositories)
{
    auto parts = loki::ConditionList {};
    for (const auto& part : condition.get_conditions())
    {
        if (const auto or_condition = std::get_if<loki::ConditionOr>(&part->get_condition()))
        {
            const auto& nested_parts = *std::get<loki::ConditionOr>(flatten(**or_condition, pddl_repositories)->get_condition());

            parts.insert(parts.end(), nested_parts.get_conditions().begin(), nested_parts.get_conditions().end());
        }
        else
        {
            parts.push_back(part);
        }
    }
    return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_or(uniquify_elements(parts)));
}

loki::Condition flatten(const loki::ConditionExistsImpl& condition, loki::PDDLRepositories& pddl_repositories)
{
    if (const auto condition_exists = std::get_if<loki::ConditionExists>(&condition.get_condition()->get_condition()))
    {
        const auto& nested_condition = *std::get<loki::ConditionExists>(flatten(**condition_exists, pddl_repositories)->get_condition());
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_exists(parameters, nested_condition.get_condition()));
    }
    return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_exists(condition.get_parameters(), condition.get_condition()));
}

loki::Condition flatten(const loki::ConditionForallImpl& condition, loki::PDDLRepositories& pddl_repositories)
{
    if (const auto condition_forall = std::get_if<loki::ConditionForall>(&condition.get_condition()->get_condition()))
    {
        const auto& nested_condition = *std::get<loki::ConditionForall>(flatten(**condition_forall, pddl_repositories)->get_condition());
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_forall(parameters, nested_condition.get_condition()));
    }
    return pddl_repositories.get_or_create_condition(pddl_repositories.get_or_create_condition_forall(condition.get_parameters(), condition.get_condition()));
}

loki::Effect flatten(const loki::EffectAndImpl& effect, loki::PDDLRepositories& pddl_repositories)
{
    auto function_to_numeric_effects = std::unordered_map<std::pair<loki::AssignOperatorEnum, loki::Function>,
                                                          loki::EffectList,
                                                          mimir::Hash<std::pair<loki::AssignOperatorEnum, loki::Function>>> {};
    auto other_effects = loki::EffectList {};

    for (const auto& part : effect.get_effects())
    {
        if (const auto and_effect = std::get_if<loki::EffectAnd>(&part->get_effect()))
        {
            const auto& nested_parts = *std::get<loki::EffectAnd>(flatten(**and_effect, pddl_repositories)->get_effect());

            for (const auto& nested_part : nested_parts.get_effects())
            {
                if (const auto numeric_effect = std::get_if<loki::EffectNumeric>(&nested_part->get_effect()))
                {
                    function_to_numeric_effects[std::make_pair((*numeric_effect)->get_assign_operator(), (*numeric_effect)->get_function())].push_back(
                        nested_part);
                }
                else
                {
                    other_effects.push_back(nested_part);
                }
            }
        }
        else
        {
            if (const auto numeric_effect = std::get_if<loki::EffectNumeric>(&part->get_effect()))
            {
                function_to_numeric_effects[std::make_pair((*numeric_effect)->get_assign_operator(), (*numeric_effect)->get_function())].push_back(part);
            }
            else
            {
                other_effects.push_back(part);
            }
        }
    }

    // Sum aggregate function expressions with same (assign_operator and function).
    for (const auto& [key, numeric_effects] : function_to_numeric_effects)
    {
        const auto& [assign_operator, function] = key;

        auto function_expressions = loki::FunctionExpressionList {};
        for (const auto& numeric_effect : numeric_effects)
        {
            function_expressions.push_back(std::get<loki::EffectNumeric>(numeric_effect->get_effect())->get_function_expression());
        }
        other_effects.push_back(pddl_repositories.get_or_create_effect(pddl_repositories.get_or_create_effect_numeric(
            assign_operator,
            function,
            pddl_repositories.get_or_create_function_expression(
                pddl_repositories.get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum::PLUS, function_expressions)))));
    }

    return pddl_repositories.get_or_create_effect(pddl_repositories.get_or_create_effect_and(uniquify_elements(other_effects)));
}

loki::Effect flatten(const loki::EffectCompositeWhenImpl& effect, loki::PDDLRepositories& pddl_repositories)
{
    if (const auto effect_when = std::get_if<loki::EffectCompositeWhen>(&effect.get_effect()->get_effect()))
    {
        const auto& nested_effect = *std::get<loki::EffectCompositeWhen>(flatten(**effect_when, pddl_repositories)->get_effect());

        return pddl_repositories.get_or_create_effect(
            pddl_repositories.get_or_create_effect_composite_when(flatten(*pddl_repositories.get_or_create_condition_and(uniquify_elements(
                                                                              loki::ConditionList { effect.get_condition(), nested_effect.get_condition() })),
                                                                          pddl_repositories),
                                                                  nested_effect.get_effect()));
    }
    return pddl_repositories.get_or_create_effect(pddl_repositories.get_or_create_effect_composite_when(effect.get_condition(), effect.get_effect()));
}

loki::Effect flatten(const loki::EffectCompositeForallImpl& effect, loki::PDDLRepositories& pddl_repositories)
{
    if (const auto effect_forall = std::get_if<loki::EffectCompositeForall>(&effect.get_effect()->get_effect()))
    {
        const auto& nested_effect = *std::get<loki::EffectCompositeForall>(flatten(**effect_forall, pddl_repositories)->get_effect());
        auto parameters = effect.get_parameters();
        const auto additional_parameters = nested_effect.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_repositories.get_or_create_effect(pddl_repositories.get_or_create_effect_composite_forall(parameters, nested_effect.get_effect()));
    }
    return pddl_repositories.get_or_create_effect(pddl_repositories.get_or_create_effect_composite_forall(effect.get_parameters(), effect.get_effect()));
}

std::string create_unique_axiom_name(Index& next_axiom_index, std::unordered_set<std::string>& simple_and_derived_predicate_names)
{
    auto axiom_name = std::string {};

    do
    {
        axiom_name = "axiom_"s + std::to_string(next_axiom_index++);
    } while (simple_and_derived_predicate_names.count(axiom_name));
    simple_and_derived_predicate_names.insert(axiom_name);

    return axiom_name;
}

}