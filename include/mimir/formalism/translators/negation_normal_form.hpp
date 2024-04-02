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

#ifndef MIMIR_FORMALISM_TRANSLATORS_NEGATION_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_NEGATION_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
/**
 * Translate formulas to negation normal form (NNF) using the following rules.
 *
 * 1. A -> B                           =>  not A or B
 * 2. A <-> B                          => (not A or B) and (A or not B)
 * 3. not (A or B)                     => not A and not B
 * 4. not (A and B)                    => not A or not B
 * 5. not (not A)                      => A
 * 6. not exists x A                   => forall x not A
 * 7. not forall x A                   => exists x not A
 * 8. A and (B and C)                  => A and B and C
 * 9. A or (B or C)                    => A or B or C
 * 10. exists(vars1, exists(vars2, A)) => exists(vars1+vars2, A)
 * 11. forall(vars1, forall(vars2, A)) => forall(vars1+vars2, A)
 */
class NNFTranslator : public BaseTranslator<NNFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<NNFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    std::unordered_map<Condition, Condition> m_translated_conditions;

    /**
     * Flatten conjunctions.
     *
     * 1. A and (B and C)  =>  A and B and C
     */
    Condition translate_flatten_conjunctions(const ConditionAndImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto and_condition = std::get_if<ConditionAndImpl>(nested_condition))
            {
                auto translated_nested_and_conditions = this->translate(and_condition->get_conditions());
                translated_nested_conditions.insert(translated_nested_conditions.end(),
                                                    translated_nested_and_conditions.begin(),
                                                    translated_nested_and_conditions.end());
            }
            else
            {
                translated_nested_conditions.push_back(this->translate(*nested_condition));
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(translated_nested_conditions);
    }

    /**
     * Flatten disjunctions.
     *
     * 1. A or (B or C)  =>  A or B or C
     */
    Condition translate_flatten_disjunctions(const ConditionOrImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto or_condition = std::get_if<ConditionOrImpl>(nested_condition))
            {
                auto translated_nested_and_conditions = this->translate(or_condition->get_conditions());
                translated_nested_conditions.insert(translated_nested_conditions.end(),
                                                    translated_nested_and_conditions.begin(),
                                                    translated_nested_and_conditions.end());
            }
            else
            {
                translated_nested_conditions.push_back(this->translate(*nested_condition));
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(translated_nested_conditions);
    }

    /**
     * Flatten existential quantifiers.
     *
     * 1. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
     */
    Condition translate_flatten_existential_quantifier(const ConditionExistsImpl& condition)
    {
        if (const auto exists_condition = std::get_if<ConditionExistsImpl>(condition.get_condition()))
        {
            auto translated_parameters = this->translate(condition.get_parameters());
            auto translated_additional_parameters = this->translate(exists_condition->get_parameters());
            translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
            return this->get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(translated_parameters,
                                                                                                     this->translate(*exists_condition->get_condition()));
        }
        return this->get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(this->translate(condition.get_parameters()),
                                                                                                 this->translate(*condition.get_condition()));
    }

    /**
     * Flatten universal quantifiers.
     *
     * 1. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
     */
    Condition translate_flatten_universal_quantifier(const ConditionForallImpl& condition)
    {
        if (const auto universal_condition = std::get_if<ConditionForallImpl>(condition.get_condition()))
        {
            auto translated_parameters = this->translate(condition.get_parameters());
            auto translated_additional_parameters = this->translate(universal_condition->get_parameters());
            translated_parameters.insert(translated_parameters.end(), translated_additional_parameters.begin(), translated_additional_parameters.end());
            return this->get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(translated_parameters,
                                                                                                     this->translate(*universal_condition->get_condition()));
        }
        return this->get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(this->translate(condition.get_parameters()),
                                                                                                 this->translate(*condition.get_condition()));
    }

    /**
     * Push negation inwards.
     *
     * 1. not (A or B)    =>  not A and not B
     * 2. not (A and B)   =>  not A or not B
     * 3. not (not A)     =>  A
     * 4. not exists x A  =>  forall x not A
     * 5. not forall x A  =>  exists x not A
     * 6. not (A -> B)    =>  not (not A or B)  =>  A and not B
     */
    Condition translate_push_negation_inwards(const ConditionNotImpl& condition)
    {
        if (const auto condition_lit = std::get_if<ConditionLiteralImpl>(condition.get_condition()))
        {
            return this->m_pddl_factories.conditions.template get_or_create<ConditionLiteralImpl>(
                this->m_pddl_factories.literals.template get_or_create<LiteralImpl>(!condition_lit->get_literal()->is_negated(),
                                                                                    this->translate(*condition_lit->get_literal()->get_atom())));
        }
        else if (const auto condition_not = std::get_if<ConditionNotImpl>(condition.get_condition()))
        {
            return this->translate(*condition_not->get_condition());
        }
        else if (const auto condition_imply = std::get_if<ConditionImplyImpl>(condition.get_condition()))
        {
            return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(ConditionList {
                this->translate(*condition_imply->get_condition_left()),
                this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition_imply->get_condition_right())) });
        }
        else if (const auto condition_and = std::get_if<ConditionAndImpl>(condition.get_condition()))
        {
            auto translated_nested_conditions = ConditionList {};
            translated_nested_conditions.reserve(condition_and->get_conditions().size());
            for (const auto& nested_condition : condition_and->get_conditions())
            {
                translated_nested_conditions.push_back(
                    this->get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(this->translate(*nested_condition)));
            }
            return this->get_pddl_factories().conditions.template get_or_create<ConditionOrImpl>(translated_nested_conditions);
        }
        else if (const auto condition_or = std::get_if<ConditionOrImpl>(condition.get_condition()))
        {
            auto translated_nested_conditions = ConditionList {};
            translated_nested_conditions.reserve(condition_or->get_conditions().size());
            for (const auto& nested_condition : condition_or->get_conditions())
            {
                translated_nested_conditions.push_back(
                    this->get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(this->translate(*nested_condition)));
            }
            return this->get_pddl_factories().conditions.template get_or_create<ConditionAndImpl>(translated_nested_conditions);
        }
        else if (const auto condition_exists = std::get_if<ConditionExistsImpl>(condition.get_condition()))
        {
            return this->get_pddl_factories().conditions.template get_or_create<ConditionForallImpl>(
                this->translate(condition_exists->get_parameters()),
                this->get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition())));
        }
        else if (const auto condition_forall = std::get_if<ConditionForallImpl>(condition.get_condition()))
        {
            return this->get_pddl_factories().conditions.template get_or_create<ConditionExistsImpl>(
                this->translate(condition_exists->get_parameters()),
                this->get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition())));
        }
        return this->get_pddl_factories().conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition()));
    }

    /**
     * Translate
     */
    Condition translate_impl(const ConditionImplyImpl& condition);
    Condition translate_impl(const ConditionNotImpl& condition);
    Condition translate_impl(const ConditionAndImpl& condition);
    Condition translate_impl(const ConditionOrImpl& condition);
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Condition translate_impl(const ConditionImpl& condition);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit NNFTranslator(PDDLFactories& pddl_factories);
};

}

#endif
