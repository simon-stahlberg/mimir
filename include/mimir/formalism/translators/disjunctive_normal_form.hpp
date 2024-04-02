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

#ifndef MIMIR_FORMALISM_TRANSLATORS_DISJUNCTIVE_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_DISJUNCTIVE_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"
#include "mimir/formalism/translators/negation_normal_form.hpp"

namespace mimir
{
/**
 * Translate formulas to disjunctive normal form (DNF) in bottom-up manner using the following rules.
 *
 * First, applies NNF translator follow by:
 *
 * 1. A and (B or C)  =>  A and B or A and C
 * 2. exists(vars, A or B)  =>  exists(vars, A) or exists(vars, B)
 * 3. forall(vars, A or B)  =>  forall(vars, A) or forall(vars, B)
 */
class DNFTranslator : public BaseTranslator<DNFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<DNFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    // Cache translations
    std::unordered_map<Condition, Condition> m_translated_conditions;

    /**
     * Apply conjunctive disjunctive distributivity.
     *
     * 1. A and (B or C)  =>  A and B or A and C
     */
    Condition translate_distributive_conjunctive_disjunctive(const ConditionAndImpl& condition)
    {
        const auto translated_parts = this->translate(condition.get_conditions());
        auto disjunctive_parts = ConditionList {};
        auto other_parts = ConditionList {};
        for (const auto part : translated_parts)
        {
            if (const auto disjunctive_part = std::get_if<ConditionOrImpl>(part))
            {
                disjunctive_parts.push_back(part);
            }
            else
            {
                other_parts.push_back(part);
            }
        }

        if (disjunctive_parts.empty())
        {
            // No disjunctive parts to distribute
            return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(translated_parts);
        }

        auto result_parts = ConditionList {};
        if (other_parts.empty())
        {
            // Immediately start with first disjunctive part
            auto part = disjunctive_parts.back();
            disjunctive_parts.pop_back();
            result_parts = ConditionList { part };
        }
        else
        {
            // Start with conjunctive part
            result_parts = ConditionList { this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(other_parts) };
        }

        while (!disjunctive_parts.empty())
        {
            auto previous_result_parts = std::move(result_parts);
            result_parts = ConditionList {};
            auto disjunctive_part_to_distribute = disjunctive_parts.back();
            const auto& current_parts = std::get_if<ConditionOrImpl>(disjunctive_part_to_distribute)->get_conditions();
            disjunctive_parts.pop_back();
            for (const auto& part1 : previous_result_parts)
            {
                for (const auto& part2 : current_parts)
                {
                    result_parts.push_back(this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(ConditionList { part1, part2 }));
                }
            }
        }

        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(result_parts);
    }

    /**
     * Translate
     */
    Condition translate_impl(const ConditionAndImpl& condition);
    Condition translate_impl(const ConditionExistsImpl& condition);
    Condition translate_impl(const ConditionForallImpl& condition);
    Condition translate_impl(const ConditionImpl& condition);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit DNFTranslator(PDDLFactories& pddl_factories);
};

}

#endif
