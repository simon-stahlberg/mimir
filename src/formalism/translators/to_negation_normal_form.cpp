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

#include "mimir/formalism/translators/to_negation_normal_form.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionImplyImpl& condition)
{
    return this->translate(*this->m_pddl_factories.get_or_create_condition_or(
        loki::ConditionList { this->m_pddl_factories.get_or_create_condition_not(condition.get_condition_left()), condition.get_condition_right() }));
}

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionNotImpl& condition)
{
    if (const auto condition_lit = std::get_if<loki::ConditionLiteralImpl>(condition.get_condition()))
    {
        return this->m_pddl_factories.get_or_create_condition_literal(
            this->m_pddl_factories.get_or_create_literal(!condition_lit->get_literal()->is_negated(), condition_lit->get_literal()->get_atom()));
    }
    else if (const auto condition_not = std::get_if<loki::ConditionNotImpl>(condition.get_condition()))
    {
        return this->translate(*condition_not->get_condition());
    }
    else if (const auto condition_imply = std::get_if<loki::ConditionImplyImpl>(condition.get_condition()))
    {
        return this->translate(*this->m_pddl_factories.get_or_create_condition_and(
            loki::ConditionList { condition_imply->get_condition_left(),
                                  this->m_pddl_factories.get_or_create_condition_not(condition_imply->get_condition_right()) }));
    }
    else if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(condition.get_condition()))
    {
        const auto flattened_condition_and = std::get_if<loki::ConditionAndImpl>(flatten_conjunctions(*condition_and, this->m_pddl_factories));
        auto nested_parts = loki::ConditionList {};
        nested_parts.reserve(flattened_condition_and->get_conditions().size());
        for (const auto& nested_condition : flattened_condition_and->get_conditions())
        {
            nested_parts.push_back(this->translate(*this->m_pddl_factories.get_or_create_condition_not(nested_condition)));
        }
        return flatten_disjunctions(*std::get_if<loki::ConditionOrImpl>(this->m_pddl_factories.get_or_create_condition_or(nested_parts)),
                                    this->m_pddl_factories);
    }
    else if (const auto condition_or = std::get_if<loki::ConditionOrImpl>(condition.get_condition()))
    {
        const auto flattened_condition_or = std::get_if<loki::ConditionOrImpl>(flatten_disjunctions(*condition_or, this->m_pddl_factories));
        auto nested_parts = loki::ConditionList {};
        nested_parts.reserve(flattened_condition_or->get_conditions().size());
        for (const auto& nested_condition : flattened_condition_or->get_conditions())
        {
            nested_parts.push_back(this->translate(*this->m_pddl_factories.get_or_create_condition_not(nested_condition)));
        }
        return flatten_conjunctions(*std::get_if<loki::ConditionAndImpl>(this->m_pddl_factories.get_or_create_condition_and(nested_parts)),
                                    this->m_pddl_factories);
    }
    else if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(condition.get_condition()))
    {
        const auto flattened_condition_exists =
            std::get_if<loki::ConditionExistsImpl>(flatten_existential_quantifier(*condition_exists, this->m_pddl_factories));
        return flatten_universal_quantifier(
            *std::get_if<loki::ConditionForallImpl>(this->m_pddl_factories.get_or_create_condition_forall(
                flattened_condition_exists->get_parameters(),
                this->translate(*this->m_pddl_factories.get_or_create_condition_not(flattened_condition_exists->get_condition())))),
            this->m_pddl_factories);
    }
    else if (const auto condition_forall = std::get_if<loki::ConditionForallImpl>(condition.get_condition()))
    {
        const auto flattened_condition_forall = std::get_if<loki::ConditionForallImpl>(flatten_universal_quantifier(*condition_forall, this->m_pddl_factories));
        return flatten_existential_quantifier(
            *std::get_if<loki::ConditionExistsImpl>(this->m_pddl_factories.get_or_create_condition_exists(
                flattened_condition_forall->get_parameters(),
                this->translate(*this->m_pddl_factories.get_or_create_condition_not(flattened_condition_forall->get_condition())))),
            this->m_pddl_factories);
    }
    throw std::runtime_error("Missing implementation to push negations inwards.");
}

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionAndImpl& condition)
{
    return flatten_conjunctions(
        *std::get_if<loki::ConditionAndImpl>(this->m_pddl_factories.get_or_create_condition_and(this->translate(condition.get_conditions()))),
        this->m_pddl_factories);
}

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionOrImpl& condition)
{
    return flatten_disjunctions(
        *std::get_if<loki::ConditionOrImpl>(this->m_pddl_factories.get_or_create_condition_or(this->translate(condition.get_conditions()))),
        this->m_pddl_factories);
}

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    return flatten_existential_quantifier(
        *std::get_if<loki::ConditionExistsImpl>(
            this->m_pddl_factories.get_or_create_condition_exists(condition.get_parameters(), this->translate(*condition.get_condition()))),
        this->m_pddl_factories);
}

loki::Condition ToNNFTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    return flatten_universal_quantifier(
        *std::get_if<loki::ConditionForallImpl>(
            this->m_pddl_factories.get_or_create_condition_forall(condition.get_parameters(), this->translate(*condition.get_condition()))),
        this->m_pddl_factories);
}

loki::Problem ToNNFTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

ToNNFTranslator::ToNNFTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
