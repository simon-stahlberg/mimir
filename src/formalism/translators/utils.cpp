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

Condition flatten_conjunctions(const ConditionAndImpl& condition, PDDLFactories& pddl_factories)
{
    auto parts = ConditionSet {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        if (const auto and_condition = std::get_if<ConditionAndImpl>(nested_condition))
        {
            const auto nested_parts = std::get_if<ConditionAndImpl>(flatten_conjunctions(*and_condition, pddl_factories));

            parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.insert(nested_condition);
        }
    }
    return pddl_factories.conditions.template get_or_create<ConditionAndImpl>(ConditionList(parts.begin(), parts.end()));
}

Condition flatten_disjunctions(const ConditionOrImpl& condition, PDDLFactories& pddl_factories)
{
    auto parts = ConditionSet {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        if (const auto or_condition = std::get_if<ConditionOrImpl>(nested_condition))
        {
            const auto nested_parts = std::get_if<ConditionOrImpl>(flatten_disjunctions(*or_condition, pddl_factories));

            parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
        }
        else
        {
            parts.insert(nested_condition);
        }
    }
    return pddl_factories.conditions.template get_or_create<ConditionOrImpl>(ConditionList(parts.begin(), parts.end()));
}

Condition flatten_existential_quantifier(const ConditionExistsImpl& condition, PDDLFactories& pddl_factories)
{
    if (const auto condition_exists = std::get_if<ConditionExistsImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<ConditionExistsImpl>(flatten_existential_quantifier(*condition_exists, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(parameters, nested_condition->get_condition());
    }
    return pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(condition.get_parameters(), condition.get_condition());
}

Condition flatten_universal_quantifier(const ConditionForallImpl& condition, PDDLFactories& pddl_factories)
{
    if (const auto condition_forall = std::get_if<ConditionForallImpl>(condition.get_condition()))
    {
        const auto nested_condition = std::get_if<ConditionForallImpl>(flatten_universal_quantifier(*condition_forall, pddl_factories));
        auto parameters = condition.get_parameters();
        const auto additional_parameters = nested_condition->get_parameters();
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
        return pddl_factories.conditions.template get_or_create<ConditionForallImpl>(parameters, nested_condition->get_condition());
    }
    return pddl_factories.conditions.template get_or_create<ConditionForallImpl>(condition.get_parameters(), condition.get_condition());
}
}