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

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

Condition NNFTranslator::translate_impl(const ConditionImplyImpl& condition)
{
    return this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(
        ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(condition.get_condition_left()),
                        condition.get_condition_right() }));
}

Condition NNFTranslator::translate_impl(const ConditionNotImpl& condition)
{
    if (const auto condition_lit = std::get_if<ConditionLiteralImpl>(condition.get_condition()))
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionLiteralImpl>(
            this->m_pddl_factories.literals.template get_or_create<LiteralImpl>(!condition_lit->get_literal()->is_negated(),
                                                                                condition_lit->get_literal()->get_atom()));
    }
    else if (const auto condition_not = std::get_if<ConditionNotImpl>(condition.get_condition()))
    {
        return this->translate(*condition_not->get_condition());
    }
    else if (const auto condition_imply = std::get_if<ConditionImplyImpl>(condition.get_condition()))
    {
        return this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(
            ConditionList { condition_imply->get_condition_left(),
                            this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(condition_imply->get_condition_right()) }));
    }
    else if (const auto condition_and = std::get_if<ConditionAndImpl>(condition.get_condition()))
    {
        const auto flattened_condition_and = std::get_if<ConditionAndImpl>(flatten_conjunctions(*condition_and, this->m_pddl_factories));
        auto nested_parts = ConditionList {};
        nested_parts.reserve(flattened_condition_and->get_conditions().size());
        for (const auto& nested_condition : flattened_condition_and->get_conditions())
        {
            nested_parts.push_back(this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(nested_condition)));
        }
        return flatten_disjunctions(*std::get_if<ConditionOrImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(nested_parts)),
                                    this->m_pddl_factories);
    }
    else if (const auto condition_or = std::get_if<ConditionOrImpl>(condition.get_condition()))
    {
        const auto flattened_condition_or = std::get_if<ConditionOrImpl>(flatten_disjunctions(*condition_or, this->m_pddl_factories));
        auto nested_parts = ConditionList {};
        nested_parts.reserve(flattened_condition_or->get_conditions().size());
        for (const auto& nested_condition : flattened_condition_or->get_conditions())
        {
            nested_parts.push_back(this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(nested_condition)));
        }
        return flatten_conjunctions(*std::get_if<ConditionAndImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(nested_parts)),
                                    this->m_pddl_factories);
    }
    else if (const auto condition_exists = std::get_if<ConditionExistsImpl>(condition.get_condition()))
    {
        const auto flattened_condition_exists = std::get_if<ConditionExistsImpl>(flatten_existential_quantifier(*condition_exists, this->m_pddl_factories));
        return flatten_universal_quantifier(
            *std::get_if<ConditionForallImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(
                flattened_condition_exists->get_parameters(),
                this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(flattened_condition_exists->get_condition())))),
            this->m_pddl_factories);
    }
    else if (const auto condition_forall = std::get_if<ConditionForallImpl>(condition.get_condition()))
    {
        const auto flattened_condition_forall = std::get_if<ConditionForallImpl>(flatten_universal_quantifier(*condition_forall, this->m_pddl_factories));
        return flatten_existential_quantifier(
            *std::get_if<ConditionExistsImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(
                flattened_condition_forall->get_parameters(),
                this->translate(*this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(flattened_condition_forall->get_condition())))),
            this->m_pddl_factories);
    }
    throw std::runtime_error("Missing implementation to push negations inwards.");
}

Condition NNFTranslator::translate_impl(const ConditionAndImpl& condition)
{
    return flatten_conjunctions(
        *std::get_if<ConditionAndImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(this->translate(condition.get_conditions()))),
        this->m_pddl_factories);
}

Condition NNFTranslator::translate_impl(const ConditionOrImpl& condition)
{
    return flatten_disjunctions(
        *std::get_if<ConditionOrImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(this->translate(condition.get_conditions()))),
        this->m_pddl_factories);
}

Condition NNFTranslator::translate_impl(const ConditionExistsImpl& condition)
{
    return flatten_existential_quantifier(*std::get_if<ConditionExistsImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(
                                              condition.get_parameters(),
                                              this->translate(*condition.get_condition()))),
                                          this->m_pddl_factories);
}

Condition NNFTranslator::translate_impl(const ConditionForallImpl& condition)
{
    return flatten_universal_quantifier(*std::get_if<ConditionForallImpl>(this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(
                                            condition.get_parameters(),
                                            this->translate(*condition.get_condition()))),
                                        this->m_pddl_factories);
}

Problem NNFTranslator::run_impl(const ProblemImpl& problem) { return this->translate(problem); }

NNFTranslator::NNFTranslator(PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
