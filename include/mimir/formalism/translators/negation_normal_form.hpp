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
 * Translate formulas to negation normal form (NNF) in top-down manner using the following rules.
 *
 * 1. A -> B                           =>  not A or B
 * 2. A <-> B                          => (not A or B) and (A or not B)
 * 3. not (A or B)                     => not A and not B
 * 4. not (A and B)                    => not A or not B
 * 5. not (not A)                      => A
 * 6. not exists x A                   => forall x not A
 * 7. not forall x A                   => exists x not A
 * 8. not (A -> B)                     => not (not A or B)                => A and not B
 * 9. A and (B and C)                  => A and B and C
 * 10. A or (B or C)                    => A or B or C
 * 11. exists(vars1, exists(vars2, A)) => exists(vars1+vars2, A)
 * 12. forall(vars1, forall(vars2, A)) => forall(vars1+vars2, A)
 */
class NNFTranslator : public BaseTranslator<NNFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<NNFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    // Cache translations
    std::unordered_map<Condition, Condition> m_translated_conditions;

    /**
     * Flatten conjunctions.
     *
     * 9. A and (B and C)  =>  A and B and C
     */
    Condition flatten_conjunctions(const ConditionAndImpl& condition)
    {
        auto parts = ConditionSet {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto and_condition = std::get_if<ConditionAndImpl>(nested_condition))
            {
                const auto nested_parts = std::get_if<ConditionAndImpl>(flatten_conjunctions(*and_condition));

                parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
            }
            else
            {
                parts.insert(nested_condition);
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(ConditionList(parts.begin(), parts.end()));
    }

    /**
     * Flatten disjunctions.
     *
     * 10. A or (B or C)  =>  A or B or C
     */
    Condition flatten_disjunctions(const ConditionOrImpl& condition)
    {
        auto parts = ConditionSet {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto or_condition = std::get_if<ConditionOrImpl>(nested_condition))
            {
                const auto nested_parts = std::get_if<ConditionOrImpl>(flatten_disjunctions(*or_condition));

                parts.insert(nested_parts->get_conditions().begin(), nested_parts->get_conditions().end());
            }
            else
            {
                parts.insert(nested_condition);
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(ConditionList(parts.begin(), parts.end()));
    }

    /**
     * Flatten existential quantifiers.
     *
     * 11. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
     */
    Condition flatten_existential_quantifier(const ConditionExistsImpl& condition)
    {
        if (const auto condition_exists = std::get_if<ConditionExistsImpl>(condition.get_condition()))
        {
            const auto nested_condition = std::get_if<ConditionExistsImpl>(flatten_existential_quantifier(*condition_exists));
            auto parameters = condition.get_parameters();
            const auto additional_parameters = nested_condition->get_parameters();
            parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
            return this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(parameters, nested_condition->get_condition());
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(condition.get_parameters(), condition.get_condition());
    }

    /**
     * Flatten universal quantifiers.
     *
     * 12. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
     */
    Condition flatten_universal_quantifier(const ConditionForallImpl& condition)
    {
        if (const auto condition_forall = std::get_if<ConditionForallImpl>(condition.get_condition()))
        {
            const auto nested_condition = std::get_if<ConditionForallImpl>(flatten_universal_quantifier(*condition_forall));
            auto parameters = condition.get_parameters();
            const auto additional_parameters = nested_condition->get_parameters();
            parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
            return this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(parameters, nested_condition->get_condition());
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(condition.get_parameters(), condition.get_condition());
    }

    /**
     * Translate
     */

    /**
     * Eliminate implication
     *
     * 1. A -> B   =>  not A or B
     * 2. A <-> B  =>  (not A or B) and (A or not B)
     */
    Condition translate_impl(const ConditionImplyImpl& condition);
    /**
     * Push negation inwards.
     *
     * 3. not (A or B)    =>  not A and not B
     * 4. not (A and B)   =>  not A or not B
     * 5. not (not A)     =>  A
     * 6. not exists x A  =>  forall x not A
     * 7. not forall x A  =>  exists x not A
     * 8. not (A -> B)    =>  not (not A or B)  =>  A and not B
     */
    Condition translate_impl(const ConditionNotImpl& condition);
    /**
     * Flatten conjunctions.
     *
     * 9. A and (B and C)  =>  A and B and C
     */
    Condition translate_impl(const ConditionAndImpl& condition);
    /**
     * Flatten disjunctions.
     *
     * 10. A or (B or C)  =>  A or B or C
     */
    Condition translate_impl(const ConditionOrImpl& condition);
    /**
     * Flatten existential quantifiers.
     *
     * 11. exists(vars1, exists(vars2, A))  =>  exists(vars1+vars2, A)
     */
    Condition translate_impl(const ConditionExistsImpl& condition);
    /**
     * Flatten universal quantifiers.
     *
     * 12. forall(vars1, forall(vars2, A))  =>  forall(vars1+vars2, A)
     */
    Condition translate_impl(const ConditionForallImpl& condition);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit NNFTranslator(PDDLFactories& pddl_factories);
};

}

#endif
