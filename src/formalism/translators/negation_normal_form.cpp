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

#include "mimir/formalism/translators/base_visitors.hpp"

namespace mimir
{

Condition NNFTranslator::translate_impl(const ConditionImplyImpl& condition)
{
    return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(
        ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition_left())),
                        this->translate(*condition.get_condition_right()) });
}

Condition NNFTranslator::translate_impl(const ConditionNotImpl& condition)
{
    return std::visit(PushInwardsConditionNotVisitor(*this), *this->translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionAndImpl& condition)
{
    auto translated_nested_conditions = ConditionList {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        auto translated_additional_nested_conditions = std::visit(FlattenConditionAndVisitor(*this), *nested_condition);
        translated_nested_conditions.insert(translated_nested_conditions.end(),
                                            translated_additional_nested_conditions.begin(),
                                            translated_additional_nested_conditions.end());
    }
    return this->m_pddl_factories.conditions.get_or_create<ConditionAndImpl>(translated_nested_conditions);
}

Condition NNFTranslator::translate_impl(const ConditionOrImpl& condition)
{
    auto translated_nested_conditions = ConditionList {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        auto translated_additional_nested_conditions = std::visit(FlattenConditionOrVisitor(*this), *nested_condition);
        translated_nested_conditions.insert(translated_nested_conditions.end(),
                                            translated_additional_nested_conditions.begin(),
                                            translated_additional_nested_conditions.end());
    }
    return this->m_pddl_factories.conditions.get_or_create<ConditionOrImpl>(translated_nested_conditions);
}

Condition NNFTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    return std::visit(FlattenConditionExistsVisitor(*this, condition), *this->translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionForallImpl& condition)
{
    return std::visit(FlattenConditionForallVisitor(*this, condition), *this->translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionImpl& condition)
{
    auto current = Condition { nullptr };

    while (true)
    {
        auto translated = std::visit(BaseTranslator::TranslateVisitor(*this), condition);

        if (current == translated)
        {
            break;
        }
        current = translated;
    }
    return current;
}

Problem NNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(problem); }

NNFTranslator::NNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
