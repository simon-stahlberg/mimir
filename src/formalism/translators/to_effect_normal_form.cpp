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

#include <loki/loki.hpp>

namespace mimir
{

loki::Effect ToENFTranslator::translate_impl(const loki::EffectAndImpl& effect)
{
    auto translated_nested_effects = this->translate(effect.get_effects());

    // Collect type information of nested effects.
    auto effects_literal = std::unordered_set<loki::Effect> {};
    auto effects_forall = std::unordered_map<loki::ParameterList, loki::EffectList, loki::UniquePDDLHasher<loki::ParameterList>> {};
    auto effects_when = loki::EffectList {};
    auto effects_other = loki::EffectList {};
    for (const auto& nested_translated_effect : translated_nested_effects)
    {
        if (const auto nested_translated_effect_literal = std::get_if<loki::EffectLiteral>(&nested_translated_effect->get_effect()))
        {
            effects_literal.emplace(nested_translated_effect);
        }
        else if (const auto nested_translated_effect_forall = std::get_if<loki::EffectCompositeForall>(&nested_translated_effect->get_effect()))
        {
            effects_forall[(*nested_translated_effect_forall)->get_parameters()].push_back((*nested_translated_effect_forall)->get_effect());
        }
        else if (const auto nested_translated_effect_when = std::get_if<loki::EffectCompositeWhen>(&nested_translated_effect->get_effect()))
        {
            effects_when.push_back(nested_translated_effect);
        }
        else
        {
            effects_other.push_back(nested_translated_effect);
        }
    }

    // 2. e and (phi > e)  =>  e
    translated_nested_effects = effects_other;
    // Effect literals always make it into the result.
    for (const auto& effect_literal : effects_literal)
    {
        translated_nested_effects.push_back(effect_literal);
    }
    auto when_condition_to_effects = std::unordered_map<loki::Condition, loki::EffectList> {};
    for (const auto& when_effect : effects_when)
    {
        const auto effect_when = std::get<loki::EffectCompositeWhen>(when_effect->get_effect());

        if (effects_literal.count(effect_when->get_effect()) == 0)
        {
            continue;  // Remove when effects that are dominated by literal effects
        }

        when_condition_to_effects[effect_when->get_condition()].push_back(effect_when->get_effect());
    }
    // 3. (phi > e1) and (phi > e2)  =>  phi > (e1 and e2)
    for (const auto& [when_condition, when_effects] : when_condition_to_effects)
    {
        translated_nested_effects.push_back(
            this->translate(*this->m_pddl_repositories.get_or_create_effect(this->m_pddl_repositories.get_or_create_effect_composite_when(
                when_condition,
                this->m_pddl_repositories.get_or_create_effect(this->m_pddl_repositories.get_or_create_effect_and(when_effects))))));
    }
    // 4. forall(vars, e1) and forall(vars, e2)  =>  forall(vars, (e1 and e2))
    for (const auto& [forall_parameters, forall_effects] : effects_forall)
    {
        translated_nested_effects.push_back(
            this->translate(*this->m_pddl_repositories.get_or_create_effect(this->m_pddl_repositories.get_or_create_effect_composite_forall(
                forall_parameters,
                this->m_pddl_repositories.get_or_create_effect(this->m_pddl_repositories.get_or_create_effect_and(forall_effects))))));
    }

    // 3. e1 and (e2 and e3)  =>  e1 and e2 and e3
    return flatten(*this->m_pddl_repositories.get_or_create_effect_and(translated_nested_effects), this->m_pddl_repositories);
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectCompositeForallImpl& effect)
{
    // 5. forall(vars1, forall(vars2, e))  =>  forall(vars1+vars2, e)
    return flatten(
        *this->m_pddl_repositories.get_or_create_effect_composite_forall(this->translate(effect.get_parameters()), this->translate(*effect.get_effect())),
        this->m_pddl_repositories);
}

loki::Effect ToENFTranslator::translate_impl(const loki::EffectCompositeWhenImpl& effect)
{
    const auto translated_condition = this->translate(*effect.get_condition());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    if (const auto translated_nested_effect_when = std::get_if<loki::EffectCompositeWhen>(&translated_nested_effect->get_effect()))
    {
        // 6. phi > (psi > e)  =>  (phi and psi) > e
        return this->translate(*this->m_pddl_repositories.get_or_create_effect(this->m_pddl_repositories.get_or_create_effect_composite_when(
            this->m_pddl_repositories.get_or_create_condition(this->m_pddl_repositories.get_or_create_condition_and(
                loki::ConditionList { translated_condition, (*translated_nested_effect_when)->get_condition() })),
            (*translated_nested_effect_when)->get_effect())));
    }
    else if (const auto translated_nested_effect_forall = std::get_if<loki::EffectCompositeForall>(&translated_nested_effect->get_effect()))
    {
        // 7. phi > forall(vars, e)  => forall(vars, phi > e)
        return this->translate(*this->m_pddl_repositories.get_or_create_effect_composite_forall(
            (*translated_nested_effect_forall)->get_parameters(),
            this->m_pddl_repositories.get_or_create_effect(
                this->m_pddl_repositories.get_or_create_effect_composite_when(translated_condition, (*translated_nested_effect_forall)->get_effect()))));
    }
    else if (const auto translated_condition_exists = std::get_if<loki::ConditionExists>(&translated_condition->get_condition()))
    {
        // 8. exists(vars, phi) > e  => forall(vars, phi > e)
        return this->translate(*this->m_pddl_repositories.get_or_create_effect_composite_forall(
            (*translated_condition_exists)->get_parameters(),
            this->m_pddl_repositories.get_or_create_effect(
                this->m_pddl_repositories.get_or_create_effect_composite_when((*translated_condition_exists)->get_condition(), translated_nested_effect))));
    }

    return this->m_pddl_repositories.get_or_create_effect(
        this->m_pddl_repositories.get_or_create_effect_composite_when(translated_condition, translated_nested_effect));
}

loki::Condition ToENFTranslator::translate_impl(const loki::ConditionAndImpl& condition)
{
    // 9. A and (B and C)  =>  A and B and C
    return flatten(*this->m_pddl_repositories.get_or_create_condition_and(this->translate(condition.get_conditions())), this->m_pddl_repositories);
}

loki::Problem ToENFTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

ToENFTranslator::ToENFTranslator(loki::PDDLRepositories& pddl_repositories) : BaseCachedRecurseTranslator(pddl_repositories) {}

}
