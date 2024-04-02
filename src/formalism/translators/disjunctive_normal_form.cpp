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

Condition DNFTranslator::translate_impl(const ConditionAndImpl& condition) { return this->translate_distributive_conjunctive_disjunctive(condition); }

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

Condition DNFTranslator::translate_impl(const ConditionImpl& condition)
{
    // Retrieve cached translations
    auto it = m_translated_conditions.find(&condition);
    if (it != m_translated_conditions.end())
    {
        return it->second;
    }

    // Translate
    auto translated = std::visit([this](auto&& arg) { return this->translate(arg); }, condition);

    // Cache translations
    m_translated_conditions.emplace(&condition, translated);

    return translated;
}

Problem DNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(*NNFTranslator(m_pddl_factories).translate(problem)); }

DNFTranslator::DNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
