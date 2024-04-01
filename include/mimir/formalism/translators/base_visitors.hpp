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

#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_VISITORS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_VISITORS_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * Push negation inwards.
 *
 * 1. not (A or B)    =>  not A and not B
 * 2. not (A and B)   =>  not A or not B
 * 3. not (not A)     =>  A
 * 4. not exists x A  =>  forall x not A
 * 5. not forall x A  =>  exists x not A
 */
template<typename Derived>
class PushInwardsConditionNotVisitor
{
private:
    BaseTranslator<Derived>& m_translator;

public:
    explicit PushInwardsConditionNotVisitor(BaseTranslator<Derived>& translator) : m_translator(translator) {}

    /// @brief Return translated ConditionNot.
    template<typename ConditionImpl>
    Condition operator()(const ConditionImpl& condition)
    {
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(condition));
    }
    Condition operator()(const ConditionLiteralImpl& condition) { return m_translator.translate(condition); }
    Condition operator()(const ConditionNotImpl& condition) { return m_translator.translate(*condition.get_condition()); }
    Condition operator()(const ConditionAndImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        translated_nested_conditions.reserve(condition.get_conditions().size());
        for (const auto& nested_condition : condition.get_conditions())
        {
            translated_nested_conditions.push_back(
                m_translator.get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*nested_condition)));
        }
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionOrImpl>(translated_nested_conditions);
    }
    Condition operator()(const ConditionOrImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        translated_nested_conditions.reserve(condition.get_conditions().size());
        for (const auto& nested_condition : condition.get_conditions())
        {
            translated_nested_conditions.push_back(
                m_translator.get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*nested_condition)));
        }
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionAndImpl>(translated_nested_conditions);
    }
    Condition operator()(const ConditionExistsImpl& condition)
    {
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(
            condition.get_parameters(),
            m_translator.get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*condition.get_condition())));
    }
    Condition operator()(const ConditionForallImpl& condition)
    {
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(
            condition.get_parameters(),
            m_translator.get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(m_translator.translate(*condition.get_condition())));
    }
};

/**
 * Apply conjunctive disjunctive distributivity.
 *
 * 1. A and (B or C)  =>  A and B or A and C
 */
template<typename Derived>
class DistributiveConditionAndOrVisitor
{
private:
    BaseTranslator<Derived>& m_translator;
    // Multipliers are all nested conditions until the currently visited element
    const ConditionList& m_multipliers;

public:
    DistributiveConditionAndOrVisitor(BaseTranslator<Derived>& translator, const ConditionList& multipliers) :
        m_translator(translator),
        m_multipliers(multipliers)
    {
    }

    /// @brief Return translated nested conditions.
    template<typename ConditionImpl>
    ConditionList operator()(const ConditionImpl& condition)
    {
        return ConditionList { m_translator.translate(condition) };
    }
    ConditionList operator()(const ConditionOrImpl& condition)
    {
        auto translated_result_conditions = ConditionList {};
        auto translated_nested_conditions = m_translator.translate(condition.get_conditions());
        for (const auto multiplier : m_multipliers)
        {
            for (const auto& nested_condition : translated_nested_conditions)
            {
                translated_result_conditions.push_back(
                    m_translator.get_pddl_factories().conditions.template get_or_create<ConditionAndImpl>(ConditionList { multiplier, nested_condition }));
            }
        }
        return translated_result_conditions;
    }
};

/**
 * Flatten existential quantifiers.
 *
 * 1. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
 */
template<typename Derived>
class FlattenConditionExistsVisitor
{
private:
    BaseTranslator<Derived>& m_translator;
    const ConditionExistsImpl& m_parent;

public:
    FlattenConditionExistsVisitor(BaseTranslator<Derived>& translator, const ConditionExistsImpl& parent) : m_translator(translator), m_parent(parent) {}

    /// @brief Return translated ConditionExists.
    template<typename ConditionImpl>
    Condition operator()(const ConditionImpl& condition)
    {
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(m_parent.get_parameters(),
                                                                                                        m_translator.translate(condition));
    }
    Condition operator()(const ConditionExistsImpl& condition)
    {
        auto translated_parameters = m_translator.translate(m_parent.get_parameters());
        auto translated_additional_parameters = m_translator.translate(condition.get_parameters());
        translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(translated_parameters,
                                                                                                        m_translator.translate(*condition.get_condition()));
    }
};

/**
 * Flatten universal quantifiers.
 *
 * 1. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
 */
template<typename Derived>
class FlattenConditionForallVisitor
{
private:
    BaseTranslator<Derived>& m_translator;
    const ConditionForallImpl& m_parent;

public:
    FlattenConditionForallVisitor(BaseTranslator<Derived>& translator, const ConditionForallImpl& parent) : m_translator(translator), m_parent(parent) {}

    /// @brief Return translated ConditionForall.
    template<typename ConditionImpl>
    Condition operator()(const ConditionImpl& condition)
    {
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(m_parent.get_parameters(),
                                                                                                        m_translator.translate(condition));
    }
    Condition operator()(const ConditionForallImpl& condition)
    {
        auto translated_parameters = m_translator.translate(m_parent.get_parameters());
        auto translated_additional_parameters = m_translator.translate(condition.get_parameters());
        translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
        return m_translator.get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(translated_parameters,
                                                                                                        m_translator.translate(*condition.get_condition()));
    }
};
}

#endif
