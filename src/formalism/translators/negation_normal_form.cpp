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

NNFTranslator::ConditionNotVisitor::ConditionNotVisitor(NNFTranslator& translator) : m_translator(translator) {}

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionLiteralImpl& condition) { return m_translator.translate(condition); }

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionNotImpl& condition) { return m_translator.translate(*condition.get_condition()); }

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionAndImpl& condition)
{
    auto translated_nested_conditions = ConditionList {};
    translated_nested_conditions.reserve(condition.get_conditions().size());
    for (const auto& nested_condition : condition.get_conditions())
    {
        translated_nested_conditions.push_back(
            m_translator.m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*nested_condition)));
    }
    return m_translator.m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(translated_nested_conditions);
}

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionOrImpl& condition)
{
    auto translated_nested_conditions = ConditionList {};
    translated_nested_conditions.reserve(condition.get_conditions().size());
    for (const auto& nested_condition : condition.get_conditions())
    {
        translated_nested_conditions.push_back(
            m_translator.m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*nested_condition)));
    }
    return m_translator.m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(translated_nested_conditions);
}

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionExistsImpl& condition)
{
    return m_translator.m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(
        condition.get_parameters(),
        m_translator.m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*condition.get_condition())));
}

Condition NNFTranslator::ConditionNotVisitor::operator()(const ConditionForallImpl& condition)
{
    return m_translator.m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(
        condition.get_parameters(),
        m_translator.m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*condition.get_condition())));
}

NNFTranslator::ConditionAndVisitor::ConditionAndVisitor(NNFTranslator& translator) : m_translator(translator) {}

ConditionList NNFTranslator::ConditionAndVisitor::operator()(const ConditionAndImpl& condition) { return m_translator.translate(condition.get_conditions()); }

NNFTranslator::ConditionOrVisitor::ConditionOrVisitor(NNFTranslator& translator) : m_translator(translator) {}

ConditionList NNFTranslator::ConditionOrVisitor::operator()(const ConditionOrImpl& condition) { return m_translator.translate(condition.get_conditions()); }

NNFTranslator::ConditionExistsVisitor::ConditionExistsVisitor(NNFTranslator& translator, const ConditionExistsImpl& parent) :
    m_translator(translator),
    m_parent(parent)
{
}

Condition NNFTranslator::ConditionExistsVisitor::operator()(const ConditionExistsImpl& condition)
{
    auto translated_parameters = m_translator.translate(m_parent.get_parameters());
    auto translated_additional_parameters = m_translator.translate(condition.get_parameters());
    translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
    return m_translator.m_pddl_factories.conditions.get_or_create<ConditionExistsImpl>(translated_parameters,
                                                                                       m_translator.translate(*condition.get_condition()));
}

NNFTranslator::ConditionForallVisitor::ConditionForallVisitor(NNFTranslator& translator, const ConditionForallImpl& parent) :
    m_translator(translator),
    m_parent(parent)
{
}

Condition NNFTranslator::ConditionForallVisitor::operator()(const ConditionForallImpl& condition)
{
    auto translated_parameters = m_translator.translate(m_parent.get_parameters());
    auto translated_additional_parameters = m_translator.translate(condition.get_parameters());
    translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
    return m_translator.m_pddl_factories.conditions.get_or_create<ConditionForallImpl>(translated_parameters,
                                                                                       m_translator.translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionImplyImpl& condition)
{
    return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(
        ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition_left())),
                        this->translate(*condition.get_condition_right()) });
}

Condition NNFTranslator::translate_impl(const ConditionNotImpl& condition)
{
    return std::visit(NNFTranslator::ConditionNotVisitor(*this), *this->translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionAndImpl& condition)
{
    auto translated_nested_conditions = ConditionList {};
    for (const auto& nested_condition : condition.get_conditions())
    {
        auto translated_additional_nested_conditions = std::visit(ConditionAndVisitor(*this), *nested_condition);
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
        auto translated_additional_nested_conditions = std::visit(ConditionOrVisitor(*this), *nested_condition);
        translated_nested_conditions.insert(translated_nested_conditions.end(),
                                            translated_additional_nested_conditions.begin(),
                                            translated_additional_nested_conditions.end());
    }
    return this->m_pddl_factories.conditions.get_or_create<ConditionOrImpl>(translated_nested_conditions);
}

Condition NNFTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    return std::visit(ConditionExistsVisitor(*this, condition), *this->translate(*condition.get_condition()));
}

Condition NNFTranslator::translate_impl(const ConditionForallImpl& condition)
{
    return std::visit(ConditionForallVisitor(*this, condition), *this->translate(*condition.get_condition()));
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
