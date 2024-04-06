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

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Condition DNFTranslator::translate_impl(const ConditionAndImpl& condition)
{
    auto disjunctive_parts = ConditionList {};
    auto other_parts = ConditionList {};
    for (const auto part : condition.get_conditions())
    {
        const auto translated_part = this->translate(*part);

        if (const auto disjunctive_part = std::get_if<ConditionOrImpl>(translated_part))
        {
            disjunctive_parts.push_back(translated_part);
        }
        else
        {
            other_parts.push_back(translated_part);
        }
    }

    if (disjunctive_parts.empty())
    {
        // No disjunctive parts to distribute
        return this->m_pddl_factories.get_or_create_condition_and(uniquify_elements(other_parts));
    }

    auto result_parts = ConditionList {};
    if (other_parts.empty())
    {
        // Immediately start with first disjunctive part
        const auto it = disjunctive_parts.begin();
        disjunctive_parts.erase(it);
        result_parts = ConditionList { *it };
    }
    else
    {
        // Start with conjunctive part
        result_parts = ConditionList { this->m_pddl_factories.get_or_create_condition_and(uniquify_elements(other_parts)) };
    }

    while (!disjunctive_parts.empty())
    {
        auto previous_result_parts = std::move(result_parts);
        result_parts = ConditionList {};
        const auto it = disjunctive_parts.begin();
        disjunctive_parts.erase(it);
        const auto& current_parts = std::get_if<ConditionOrImpl>(*it)->get_conditions();
        for (const auto& part1 : previous_result_parts)
        {
            for (const auto& part2 : current_parts)
            {
                result_parts.push_back(
                    flatten_conjunctions(*std::get_if<ConditionAndImpl>(this->m_pddl_factories.get_or_create_condition_and(ConditionList { part1, part2 })),
                                         this->m_pddl_factories));
            }
        }
    }

    return this->m_pddl_factories.get_or_create_condition_or(uniquify_elements(result_parts));
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
            result_parts.push_back(this->m_pddl_factories.get_or_create_condition_exists(translated_parameters, part));
        }
        return this->m_pddl_factories.get_or_create_condition_or(result_parts);
    }
    return this->m_pddl_factories.get_or_create_condition_exists(translated_parameters, translated_condition);
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
            result_parts.push_back(this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, part));
        }
        return this->m_pddl_factories.get_or_create_condition_or(result_parts);
    }
    return this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, translated_condition);
}

Problem DNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(*NNFTranslator(m_pddl_factories).translate(problem)); }

DNFTranslator::DNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
