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
    auto translated_nested_effects = this->translate(effect.get_effects());

    // Collect type information of nested effects.
    auto effect_literals = std::unordered_set<loki::Effect> {};
    auto effect_when = loki::EffectList {};
    auto other_effects = loki::EffectList {};
    for (const auto& nested_translated_effect : translated_nested_effects)
    {
        if (const auto nested_translated_effect_literal = std::get_if<loki::EffectLiteralImpl>(nested_translated_effect))
        {
            effect_literals.emplace(nested_translated_effect);
        }
        else if (const auto nested_translated_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(nested_translated_effect))
        {
            effect_when.push_back(nested_translated_effect);
        }
        else
        {
            other_effects.push_back(nested_translated_effect);
        }
    }

    // 2. e and (phi > e)  =>  e
    translated_nested_effects = other_effects;
    // Effect literals always make it into the result.
    for (const auto& effect_literal : effect_literals)
    {
        translated_nested_effects.push_back(effect_literal);
    }
    // Remove when effects that are dominated by literal effects
    for (const auto& part : effect_when)
    {
        if (effect_literals.count(std::get<loki::EffectConditionalWhenImpl>(*part).get_effect()) == 0)
        {
            translated_nested_effects.push_back(part);
        }
    }

    // 3. e1 and (e2 and e3)  =>  e1 and e2 and e3
    auto result = flatten(std::get<loki::EffectAndImpl>(*this->m_pddl_factories.get_or_create_effect_and(translated_nested_effects)), this->m_pddl_factories);

    // Stabilize
    if (&effect != &std::get<loki::EffectAndImpl>(*result))
    {
        result = this->translate(*result);
    }

    return result;
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    auto translated_parameters = this->translate(effect.get_parameters());
    auto translated_nested_effect = this->translate(*effect.get_effect());

    if (const auto translated_nested_effect_and = std::get_if<loki::EffectAndImpl>(translated_nested_effect))
    {
        // 4. forall(vars, (e1 and e2))  => forall(vars, e1) and forall(vars, e2)
        auto result_parts = loki::EffectList {};
        for (const auto& part : translated_nested_effect_and->get_effects())
        {
            result_parts.push_back(this->m_pddl_factories.get_or_create_effect_conditional_forall(translated_parameters, part));
        }
        return this->translate(*this->m_pddl_factories.get_or_create_effect_and(result_parts));
    }
    // 5. forall(vars1, forall(vars2, e))  =>  forall(vars1+vars2, e)
    auto result = flatten(std::get<loki::EffectConditionalForallImpl>(
                              *this->m_pddl_factories.get_or_create_effect_conditional_forall(translated_parameters, translated_nested_effect)),
                          this->m_pddl_factories);

    // Stabilize
    if (&effect != &std::get<loki::EffectConditionalForallImpl>(*result))
    {
        result = this->translate(*result);
    }

    return result;
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalWhenImpl& effect)
{
    const auto translated_condition = this->translate(*effect.get_condition());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    if (const auto translated_nested_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(translated_nested_effect))
    {
        // 6. phi > (psi > e)  =>  (phi and psi) > e
        return this->translate(*this->m_pddl_factories.get_or_create_effect_conditional_when(
            this->m_pddl_factories.get_or_create_condition_and(loki::ConditionList { translated_condition, translated_nested_effect_when->get_condition() }),
            translated_nested_effect_when->get_effect()));
    }
    else if (const auto translated_nested_effect_and = std::get_if<loki::EffectAndImpl>(effect.get_effect()))
    {
        // 7. phi > (e1 and e2)  =>  (phi > e1) and (phi > e2)
        auto parts = loki::EffectList {};
        for (const auto& translated_nested_effect : translated_nested_effect_and->get_effects())
        {
            parts.push_back(this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_nested_effect));
        }
        return this->translate(*this->m_pddl_factories.get_or_create_effect_and(parts));
    }
    else if (const auto translated_nested_effect_forall = std::get_if<loki::EffectConditionalForallImpl>(effect.get_effect()))
    {
        // 8. phi > forall(vars, e)  => forall(vars, phi > e)
        return this->translate(*this->m_pddl_factories.get_or_create_effect_conditional_forall(
            translated_nested_effect_forall->get_parameters(),
            this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_nested_effect_forall->get_effect())));
    }
    else if (const auto translated_condition_exists = std::get_if<loki::ConditionExistsImpl>(translated_condition))
    {
        // 9. exists(vars, phi) > e  => forall(vars, phi > e)
        return this->translate(*this->m_pddl_factories.get_or_create_effect_conditional_forall(
            translated_condition_exists->get_parameters(),
            this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition_exists->get_condition(), translated_nested_effect)));
    }

    auto result = this->m_pddl_factories.get_or_create_effect_conditional_when(translated_condition, translated_nested_effect);

    // Stabilize
    if (&effect != &std::get<loki::EffectConditionalWhenImpl>(*result))
    {
        result = this->translate(*result);
    }

    return result;
}

loki::Condition ToENFTranslator::translate_impl(const loki::ConditionAndImpl& condition)
{
    // 10. A and (B and C)  =>  A and B and C
    return flatten(*std::get_if<loki::ConditionAndImpl>(this->m_pddl_factories.get_or_create_condition_and(this->translate(condition.get_conditions()))),
                   this->m_pddl_factories);
}

loki::Problem ToENFTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

ToENFTranslator::ToENFTranslator(loki::PDDLFactories& pddl_factories) : BaseCachedRecurseTranslator(pddl_factories) {}

}
