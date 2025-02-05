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

#include "mimir/formalism/translators/initialize_metric.hpp"

#include "mimir/common/collections.hpp"

namespace mimir
{

static loki::FunctionSkeleton get_or_create_total_cost_function_skeleton(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_function_skeleton("total-cost",
                                                             loki::ParameterList {},
                                                             pddl_repositories.get_or_create_type("number", loki::TypeList {}));
}

static loki::Function get_or_create_total_cost_function(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_function(get_or_create_total_cost_function_skeleton(pddl_repositories), loki::TermList {});
}

static loki::FunctionExpression get_or_create_total_cost_function_expression(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_function_expression(
        pddl_repositories.get_or_create_function_expression_function(get_or_create_total_cost_function(pddl_repositories)));
}

static loki::FunctionExpression get_or_create_constant_one_function_expression(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_function_expression(pddl_repositories.get_or_create_function_expression_number(1));
}

static loki::OptimizationMetric get_or_create_total_cost_metric(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_optimization_metric(loki::OptimizationMetricEnum::MINIMIZE,
                                                               get_or_create_total_cost_function_expression(pddl_repositories));
}

static loki::Effect get_or_create_total_cost_numeric_effect(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_effect(
        pddl_repositories.get_or_create_effect_numeric(loki::AssignOperatorEnum::INCREASE,
                                                       get_or_create_total_cost_function(pddl_repositories),
                                                       get_or_create_constant_one_function_expression(pddl_repositories)));
}

static loki::FunctionValue get_or_create_initial_total_cost_function_value(loki::PDDLRepositories& pddl_repositories)
{
    return pddl_repositories.get_or_create_function_value(get_or_create_total_cost_function(pddl_repositories), 0);
}

void InitializeMetricTranslator::prepare_impl(loki::Domain domain)
{
    m_action_costs_enabled = domain->get_requirements()->test(loki::RequirementEnum::ACTION_COSTS);
}

void InitializeMetricTranslator::prepare_impl(loki::Problem problem)
{
    this->prepare(problem->get_domain());

    m_has_metric_defined = (problem->get_optimization_metric().has_value());
}

loki::Effect InitializeMetricTranslator::translate_impl(loki::Effect effect)
{
    if (!m_has_metric_defined && !m_action_costs_enabled)
    {
        if (const auto& effect_and = std::get_if<loki::EffectAnd>(&effect->get_effect()))
        {
            auto parts = (*effect_and)->get_effects();

            parts.push_back(get_or_create_total_cost_numeric_effect(m_pddl_repositories));

            return m_pddl_repositories.get_or_create_effect(m_pddl_repositories.get_or_create_effect_and(parts));
        }
        else
        {
            return m_pddl_repositories.get_or_create_effect(
                m_pddl_repositories.get_or_create_effect_and(loki::EffectList { effect, get_or_create_total_cost_numeric_effect(m_pddl_repositories) }));
        }
    }

    return effect;
}

loki::Domain InitializeMetricTranslator::translate_impl(loki::Domain domain)
{
    auto translated_functions = this->translate(domain->get_functions());
    translated_functions.push_back(get_or_create_total_cost_function_skeleton(m_pddl_repositories));

    return this->m_pddl_repositories.get_or_create_domain(domain->get_filepath(),
                                                          domain->get_name(),
                                                          this->translate(domain->get_requirements()),
                                                          this->translate(domain->get_types()),
                                                          this->translate(domain->get_constants()),
                                                          this->translate(domain->get_predicates()),
                                                          uniquify_elements(translated_functions),
                                                          this->translate(domain->get_actions()),
                                                          this->translate(domain->get_axioms()));
}

loki::Problem InitializeMetricTranslator::translate_impl(loki::Problem problem)
{
    auto translated_domain = this->translate(problem->get_domain());

    auto translated_function_values = this->translate(problem->get_function_values());
    if (!m_has_metric_defined)
    {
        translated_function_values.push_back(get_or_create_initial_total_cost_function_value(m_pddl_repositories));
    }

    auto translated_metric =
        m_has_metric_defined ? this->translate(problem->get_optimization_metric().value()) : get_or_create_total_cost_metric(m_pddl_repositories);

    return this->m_pddl_repositories.get_or_create_problem(
        problem->get_filepath(),
        translated_domain,
        problem->get_name(),
        this->translate(problem->get_requirements()),
        this->translate(problem->get_objects()),
        this->translate(problem->get_derived_predicates()),
        this->translate(problem->get_initial_literals()),
        translated_function_values,
        (problem->get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(problem->get_goal_condition().value())) : std::nullopt),
        translated_metric,
        this->translate(problem->get_axioms()));
}

InitializeMetricTranslator::InitializeMetricTranslator(loki::PDDLRepositories& pddl_repositories) : BaseCachedRecurseTranslator(pddl_repositories) {}
}