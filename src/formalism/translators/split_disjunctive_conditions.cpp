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

#include "mimir/formalism/translators/split_disjunctive_conditions.hpp"

#include "mimir/formalism/translators/utils.hpp"

using namespace std::string_literals;

namespace mimir
{

static loki::ActionList split_actions_at_disjunction(const loki::ActionList& actions, loki::PDDLFactories& pddl_factories)
{
    auto split_actions = loki::ActionList {};

    for (const auto& action : actions)
    {
        auto condition = action->get_condition();
        if (condition.has_value() && std::holds_alternative<loki::ConditionOrImpl>(*condition.value()))
        {
            for (const auto& part : std::get<loki::ConditionOrImpl>(*condition.value()).get_conditions())
            {
                split_actions.push_back(pddl_factories.get_or_create_action(action->get_name(),
                                                                            action->get_original_arity(),
                                                                            action->get_parameters(),
                                                                            part,
                                                                            action->get_effect()));
            }
        }
        else
        {
            split_actions.push_back(action);
        }
    }
    return uniquify_elements(split_actions);
}

static loki::AxiomList split_axioms_at_disjunction(const loki::AxiomList& axioms, loki::PDDLFactories& pddl_factories)
{
    auto split_axioms = loki::AxiomList {};

    for (const auto& axiom : axioms)
    {
        auto condition = axiom->get_condition();
        if (condition && std::holds_alternative<loki::ConditionOrImpl>(*condition))
        {
            for (const auto& part : std::get<loki::ConditionOrImpl>(*condition).get_conditions())
            {
                split_axioms.push_back(pddl_factories.get_or_create_axiom(axiom->get_derived_predicate_name(),
                                                                          axiom->get_parameters(),
                                                                          part,
                                                                          axiom->get_num_parameters_to_ground_head()));
            }
        }
        else
        {
            split_axioms.push_back(axiom);
        }
    }
    return uniquify_elements(split_axioms);
}

loki::Effect SplitDisjunctiveConditionsTranslator::translate_impl(const loki::EffectConditionalWhenImpl& effect)
{
    const auto& condition = effect.get_condition();
    if (condition && std::holds_alternative<loki::ConditionOrImpl>(*condition))
    {
        auto split_effects = loki::EffectList {};
        for (const auto& part : std::get<loki::ConditionOrImpl>(*condition).get_conditions())
        {
            split_effects.push_back(
                this->m_pddl_factories.get_or_create_effect_conditional_when(this->translate(*part), this->translate(*effect.get_effect())));
        }
        return this->m_pddl_factories.get_or_create_effect_and(split_effects);
    }
    else
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_when(this->translate(*effect.get_condition()), this->translate(*effect.get_effect()));
    }
}

loki::Domain SplitDisjunctiveConditionsTranslator::translate_impl(const loki::DomainImpl& domain)
{
    // Split actions
    auto translated_actions = split_actions_at_disjunction(this->translate(domain.get_actions()), this->m_pddl_factories);

    // Split axioms
    auto translated_axioms = split_axioms_at_disjunction(this->translate(domain.get_axioms()), this->m_pddl_factories);

    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->translate(*domain.get_requirements()),
                                                       this->translate(domain.get_types()),
                                                       this->translate(domain.get_constants()),
                                                       this->translate(domain.get_predicates()),
                                                       this->translate(domain.get_functions()),
                                                       translated_actions,
                                                       translated_axioms);
}

loki::Problem SplitDisjunctiveConditionsTranslator::translate_impl(const loki::ProblemImpl& problem)
{
    // Split axioms
    auto translated_axioms = split_axioms_at_disjunction(this->translate(problem.get_axioms()), this->m_pddl_factories);

    return this->m_pddl_factories.get_or_create_problem(
        this->translate(*problem.get_domain()),
        problem.get_name(),
        this->translate(*problem.get_requirements()),
        this->translate(problem.get_objects()),
        this->translate(problem.get_derived_predicates()),
        this->translate(problem.get_initial_literals()),
        this->translate(problem.get_numeric_fluents()),
        (problem.get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(*problem.get_goal_condition().value())) : std::nullopt),
        (problem.get_optimization_metric().has_value() ? std::optional<loki::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                         std::nullopt),
        translated_axioms);
}

loki::Problem SplitDisjunctiveConditionsTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

SplitDisjunctiveConditionsTranslator::SplitDisjunctiveConditionsTranslator(loki::PDDLFactories& pddl_factories) :
    BaseCachedRecurseTranslator<SplitDisjunctiveConditionsTranslator>(pddl_factories)
{
}
}
