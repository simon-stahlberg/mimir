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

#include "mimir/formalism/translators/disjunctive_normal_form.hpp"

namespace mimir
{

Condition DNFTranslator::translate_impl(const ConditionAndImpl& condition)
{
    const auto translated_parts = this->translate(condition.get_conditions());
    auto disjunctive_parts = ConditionList {};
    auto other_parts = ConditionList {};
    for (const auto part : translated_parts)
    {
        if (const auto disjunctive_part = std::get_if<ConditionOrImpl>(part))
        {
            disjunctive_parts.push_back(part);
        }
        else
        {
            other_parts.push_back(part);
        }
    }

    if (disjunctive_parts.empty())
    {
        // No disjunctive parts to distribute
        return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(translated_parts);
    }

    auto result_parts = ConditionList {};
    if (other_parts.empty())
    {
        // Immediately start with first disjunctive part
        auto part = disjunctive_parts.back();
        disjunctive_parts.pop_back();
        result_parts = ConditionList { part };
    }
    else
    {
        // Start with conjunctive part
        result_parts = ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(other_parts) };
    }

    while (!disjunctive_parts.empty())
    {
        auto previous_result_parts = std::move(result_parts);
        result_parts = ConditionList {};
        auto disjunctive_part_to_distribute = disjunctive_parts.back();
        const auto& current_parts = std::get_if<ConditionOrImpl>(disjunctive_part_to_distribute)->get_conditions();
        disjunctive_parts.pop_back();
        for (const auto& part1 : previous_result_parts)
        {
            for (const auto& part2 : current_parts)
            {
                result_parts.push_back(this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(ConditionList { part1, part2 }));
            }
        }
    }

    return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(result_parts);
}

Condition DNFTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_condition = this->translate(*condition.get_condition());
    if (const auto translated_disjunctive_condition = std::get_if<ConditionOrImpl>(translated_condition))
    {
        auto result_parts = ConditionList {};
        for (const auto& part : translated_disjunctive_condition->get_conditions())
        {
            result_parts.push_back(this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(translated_parameters, part));
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(result_parts);
    }
    return this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(translated_parameters, translated_condition);
}

Condition DNFTranslator::translate_impl(const ConditionForallImpl& condition)
{
    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_condition = this->translate(*condition.get_condition());
    if (const auto translated_disjunctive_condition = std::get_if<ConditionOrImpl>(translated_condition))
    {
        auto result_parts = ConditionList {};
        for (const auto& part : translated_disjunctive_condition->get_conditions())
        {
            result_parts.push_back(this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(translated_parameters, part));
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(result_parts);
    }
    return this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(translated_parameters, translated_condition);
}

Problem DNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(*NNFTranslator(m_pddl_factories).translate(problem)); }

DNFTranslator::DNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
