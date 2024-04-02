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

#include "mimir/formalism/translators/negation_normal_form.hpp"

namespace mimir
{

Condition NNFTranslator::translate_impl(const ConditionImplyImpl& condition)
{
    return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(
        ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition_left())),
                        this->translate(*condition.get_condition_right()) });
}

Condition NNFTranslator::translate_impl(const ConditionNotImpl& condition) { return this->translate_push_negation_inwards(condition); }

Condition NNFTranslator::translate_impl(const ConditionAndImpl& condition) { return this->translate_flatten_conjunctions(condition); }

Condition NNFTranslator::translate_impl(const ConditionOrImpl& condition) { return this->translate_flatten_disjunctions(condition); }

Condition NNFTranslator::translate_impl(const ConditionExistsImpl& condition) { return this->translate_flatten_existential_quantifier(condition); }

Condition NNFTranslator::translate_impl(const ConditionForallImpl& condition) { return this->translate_flatten_universal_quantifier(condition); }

Condition NNFTranslator::translate_impl(const ConditionImpl& condition)
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

Problem NNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(problem); }

NNFTranslator::NNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
