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

#include "mimir/formalism/translators/rename_quantified_variables.hpp"

using namespace std::string_literals;

namespace mimir
{

static loki::Variable create_renamed_variable(const loki::Variable& variable, size_t num_quantification, loki::PDDLFactories& pddl_factories)
{
    return pddl_factories.get_or_create_variable(variable->get_name() + "_" + std::to_string(variable->get_identifier()) + "_"
                                                 + std::to_string(num_quantification));
}

void RenameQuantifiedVariablesTranslator::rename_variables(const loki::ParameterList& parameters)
{
    // Allow renaming nested variables
    m_renaming_enabled = true;

    for (const auto& parameter : parameters)
    {
        // Increment number of quantifications of the variable and rename it.
        const auto renamed_variable =
            create_renamed_variable(parameter->get_variable(), ++m_num_quantifications.at(parameter->get_variable()), this->m_pddl_factories);

        m_renamings[parameter->get_variable()] = renamed_variable;
    }
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::VariableImpl& variable) { m_variables.insert(&variable); }

loki::Variable RenameQuantifiedVariablesTranslator::translate_impl(const loki::VariableImpl& variable)
{
    return (m_renaming_enabled) ? m_renamings[&variable] : m_pddl_factories.get_or_create_variable(variable.get_name());
}

loki::Predicate RenameQuantifiedVariablesTranslator::translate_impl(const loki::PredicateImpl& predicate)
{
    // Disallow renaming of nested variables
    m_renaming_enabled = false;

    auto result = this->m_pddl_factories.get_or_create_predicate(predicate.get_name(), this->translate(predicate.get_parameters()));

    // Allow renaming again
    m_renaming_enabled = true;

    return result;
}

loki::FunctionSkeleton RenameQuantifiedVariablesTranslator::translate_impl(const loki::FunctionSkeletonImpl& function_skeleton)
{
    // Disallow renaming of nested variables
    m_renaming_enabled = false;

    auto result = this->m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(),
                                                                         this->translate(function_skeleton.get_parameters()),
                                                                         this->translate(*function_skeleton.get_type()));

    // Allow renaming again
    m_renaming_enabled = true;

    return result;
}

loki::Action RenameQuantifiedVariablesTranslator::translate_impl(const loki::ActionImpl& action)
{
    rename_variables(action.get_parameters());

    const auto translated_parameters = this->translate(action.get_parameters());
    const auto translated_conditions =
        (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt);
    const auto translated_effect =
        (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt);

    return this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                       action.get_original_arity(),
                                                       translated_parameters,
                                                       translated_conditions,
                                                       translated_effect);
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    rename_variables(condition.get_parameters());

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    return this->m_pddl_factories.get_or_create_condition_exists(translated_parameters, translated_nested_condition);
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    rename_variables(condition.get_parameters());

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    return this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, translated_nested_condition);
}

loki::Effect RenameQuantifiedVariablesTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    rename_variables(effect.get_parameters());

    const auto translated_parameters = this->translate(effect.get_parameters());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    return this->m_pddl_factories.get_or_create_effect_conditional_forall(translated_parameters, translated_nested_effect);
}

loki::Problem RenameQuantifiedVariablesTranslator::run_impl(const loki::ProblemImpl& problem)
{
    this->prepare(problem);

    // Initialize
    for (const auto& variable : m_variables)
    {
        m_num_quantifications.emplace(variable, 0);

        const auto renamed_variable = create_renamed_variable(variable, 0, this->m_pddl_factories);

        m_renamings.emplace(variable, renamed_variable);
    }

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
        this->translate(problem.get_axioms()));
}

RenameQuantifiedVariablesTranslator::RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories) :
    BaseRecurseTranslator(pddl_factories),
    m_renaming_enabled(true)
{
}

}
