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

loki::Condition flatten(const loki::ConditionAndImpl& condition, loki::PDDLFactories& pddl_factories)
{
    auto parts = loki::ConditionList {};
    for (const auto& part : condition.get_conditions())
    {
        if (const auto and_condition = std::get_if<loki::ConditionAndImpl>(part))
        {
            const auto& nested_parts = std::get<loki::ConditionAndImpl>(*flatten(*and_condition, pddl_factories));

            parts.insert(parts.end(), nested_parts.get_conditions().begin(), nested_parts.get_conditions().end());
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
    // std::cout << "flatten: " << effect << std::endl;
    auto parts = loki::EffectList {};
    for (const auto& part : effect.get_effects())
    {
        // std::cout << "nested effect: " << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

        if (const auto and_effect = std::get_if<loki::EffectAndImpl>(part))
        {
            // std::cout << "found nested and in flatten" << std::endl;

            const auto& nested_parts = std::get<loki::EffectAndImpl>(*flatten(*and_effect, pddl_factories));

            parts.insert(parts.end(), nested_parts.get_effects().begin(), nested_parts.get_effects().end());
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
            const auto& nested_parts = std::get<loki::ConditionOrImpl>(*flatten(*or_condition, pddl_factories));

            parts.insert(parts.end(), nested_parts.get_conditions().begin(), nested_parts.get_conditions().end());
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
        const auto& nested_condition = std::get<loki::ConditionExistsImpl>(*flatten(*condition_exists, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.get_or_create_condition_exists(parameters, nested_condition.get_condition());
    }
    return pddl_factories.get_or_create_condition_exists(condition.get_parameters(), condition.get_condition());
}

loki::Condition flatten(const loki::ConditionForallImpl& condition, loki::PDDLFactories& pddl_factories)
{
    if (const auto condition_forall = std::get_if<loki::ConditionForallImpl>(condition.get_condition()))
    {
        const auto& nested_condition = std::get<loki::ConditionForallImpl>(*flatten(*condition_forall, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.get_or_create_condition_forall(parameters, nested_condition.get_condition());
    }
    return pddl_factories.get_or_create_condition_forall(condition.get_parameters(), condition.get_condition());
}

loki::Effect flatten(const loki::EffectConditionalWhenImpl& effect, loki::PDDLFactories& pddl_factories)
{
    if (const auto effect_when = std::get_if<loki::EffectConditionalWhenImpl>(effect.get_effect()))
    {
        const auto& nested_effect = std::get<loki::EffectConditionalWhenImpl>(*flatten(*effect_when, pddl_factories));

        return pddl_factories.get_or_create_effect_conditional_when(
            flatten(*std::get_if<loki::ConditionAndImpl>(pddl_factories.get_or_create_condition_and(
                        uniquify_elements(loki::ConditionList { effect.get_condition(), nested_effect.get_condition() }))),
                    pddl_factories),
            nested_effect.get_effect());
    }
    return pddl_factories.get_or_create_effect_conditional_when(effect.get_condition(), effect.get_effect());
}

loki::Effect flatten(const loki::EffectConditionalForallImpl& effect, loki::PDDLFactories& pddl_factories)
{
    if (const auto effect_forall = std::get_if<loki::EffectConditionalForallImpl>(effect.get_effect()))
    {
        const auto& nested_effect = std::get<loki::EffectConditionalForallImpl>(*flatten(*effect_forall, pddl_factories));
        auto parameters = effect.get_parameters();
        const auto additional_parameters = nested_effect.get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.get_or_create_effect_conditional_forall(parameters, nested_effect.get_effect());
    }
    return pddl_factories.get_or_create_effect_conditional_forall(effect.get_parameters(), effect.get_effect());
}

std::string create_unique_axiom_name(uint64_t& next_axiom_id, std::unordered_set<std::string>& simple_and_derived_predicate_names)
{
    auto axiom_name = std::string {};

    do
    {
        axiom_name = "axiom_"s + std::to_string(next_axiom_id++);
    } while (simple_and_derived_predicate_names.count(axiom_name));
    simple_and_derived_predicate_names.insert(axiom_name);

    return axiom_name;
}

}