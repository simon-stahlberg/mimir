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

#include "mimir/formalism/translators/to_effect_normal_form.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

loki::Effect ToENFTranslator::translate_impl(const loki::EffectAndImpl& effect)
{
    auto translated_effects = this->translate(effect.get_effects());

    // Collect type information of nested effects.
    auto effect_literals = std::unordered_set<loki::Effect> {};
    auto nested_effect_when_partitioning = std::unordered_map<loki::Effect, loki::EffectList> {};
    auto other_effects = loki::EffectList {};
    for (const auto& nested_translated_effect : translated_effects)
    {
        if (const auto nested_translated_effect_literal = std::get_if<loki::EffectLiteralImpl>(nested_translated_effect))
        {
            effect_literals.emplace(nested_translated_effect);
        }
        if (const auto nested_translated_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(nested_translated_effect))
        {
            nested_effect_when_partitioning[nested_translated_effect_when->get_effect()].push_back(nested_translated_effect);
        }
        else
        {
            other_effects.push_back(nested_translated_effect);
        }
    }

    // 3. (c1 > e) and (c2 > e)  =>  (c1 or c2) > e
    auto effect_when = loki::EffectList {};
    for (const auto& [nested_effect, partitioning] : nested_effect_when_partitioning)
    {
        assert(partitioning.size() > 0);
        if (partitioning.size() == 1)
        {
            effect_when.push_back(nested_effect);
        }
        else
        {
            auto parts = loki::ConditionList {};
            for (const auto& part : partitioning)
            {
                parts.push_back(std::get_if<loki::EffectConditionalWhenImpl>(part)->get_condition());
            }

            effect_when.push_back(this->m_pddl_factories.get_or_create_effect_conditional_when(
                flatten(*std::get_if<loki::ConditionOrImpl>(this->m_pddl_factories.get_or_create_condition_or(parts)), this->m_pddl_factories),
                nested_effect));
        }
    }

    // 4. e and (c > e)          =>  e
    translated_effects = other_effects;
    // Effect literals always make it into the result.
    for (const auto& effect_literal : effect_literals)
    {
        translated_effects.push_back(effect_literal);
    }
    // Remove when effects that are dominated by literal effects
    for (const auto& effect_when : effect_when)
    {
        if (effect_literals.count(std::get_if<loki::EffectConditionalWhenImpl>(effect_when)->get_effect()) == 0)
        {
            translated_effects.push_back(effect_when);
        }
    }

    // 8. e1 and (e2 and e3)     =>  e1 and e2 and e3
    return flatten(*std::get_if<loki::EffectAndImpl>(this->m_pddl_factories.get_or_create_effect_and(translated_effects)), this->m_pddl_factories);
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    return flatten(
        *std::get_if<loki::EffectConditionalForallImpl>(
            this->m_pddl_factories.get_or_create_effect_conditional_forall(this->translate(effect.get_parameters()), this->translate(*effect.get_effect()))),
        this->m_pddl_factories);
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalWhenImpl& effect)
{
    const auto translated_condition = this->translate(*effect.get_condition());
    const auto translated_effect = this->translate(*effect.get_effect());
    if (const auto translated_nested_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(translated_effect))
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_when(
            flatten(*std::get_if<loki::ConditionAndImpl>(this->m_pddl_factories.get_or_create_condition_and(
                        uniquify_elements(loki::ConditionList { translated_condition, translated_nested_effect_when->get_condition() }))),
                    this->m_pddl_factories),
            translated_nested_effect_when->get_effect());
    }
    else if (const auto translated_nested_effect_and = std::get_if<loki::EffectAndImpl>(effect.get_effect()))
    {
        auto parts = loki::EffectList {};
        for (const auto& translated_nested_effect : translated_nested_effect_and->get_effects())
        {
            parts.push_back(this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_nested_effect));
        }
        return flatten(*std::get_if<loki::EffectAndImpl>(this->m_pddl_factories.get_or_create_effect_and(parts)), this->m_pddl_factories);
    }

    return this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_effect);
}

loki::Problem ToENFTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

ToENFTranslator::ToENFTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
