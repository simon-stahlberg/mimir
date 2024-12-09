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

static void initialize_num_quantifications(const loki::VariableSet& variables, std::unordered_map<loki::Variable, size_t>& out_num_quantifications)
{
    out_num_quantifications.clear();
    for (const auto& variable : variables)
    {
        out_num_quantifications.emplace(variable, 0);
    }
}

static void increment_num_quantifications(const loki::ParameterList& parameters, std::unordered_map<loki::Variable, size_t>& ref_num_quantifications)
{
    for (const auto& parameter : parameters)
    {
        ++ref_num_quantifications.at(parameter->get_variable());
    }
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::VariableImpl& variable) { m_variables.insert(&variable); }

loki::Variable RenameQuantifiedVariablesTranslator::translate_impl(const loki::VariableImpl& variable)
{
    return (m_renaming_enabled) ? m_pddl_repositories.get_or_create_variable(variable.get_name() + "_" + std::to_string(variable.get_index()) + "_"
                                                                             + std::to_string(m_num_quantifications.at(&variable))) :
                                  m_pddl_repositories.get_or_create_variable(variable.get_name());
}

loki::Predicate RenameQuantifiedVariablesTranslator::translate_impl(const loki::PredicateImpl& predicate)
{
    // Disallow renaming of nested variables
    m_renaming_enabled = false;

    auto result = this->m_pddl_repositories.get_or_create_predicate(predicate.get_name(), this->translate(predicate.get_parameters()));

    // Allow renaming again
    m_renaming_enabled = true;

    return result;
}

loki::FunctionSkeleton RenameQuantifiedVariablesTranslator::translate_impl(const loki::FunctionSkeletonImpl& function_skeleton)
{
    // Disallow renaming of nested variables
    m_renaming_enabled = false;

    auto result = this->m_pddl_repositories.get_or_create_function_skeleton(function_skeleton.get_name(),
                                                                            this->translate(function_skeleton.get_parameters()),
                                                                            this->translate(*function_skeleton.get_type()));

    // Allow renaming again
    m_renaming_enabled = true;

    return result;
}

loki::Action RenameQuantifiedVariablesTranslator::translate_impl(const loki::ActionImpl& action)
{
    initialize_num_quantifications(m_variables, m_num_quantifications);
    increment_num_quantifications(action.get_parameters(), m_num_quantifications);

    const auto translated_parameters = this->translate(action.get_parameters());
    const auto translated_conditions =
        (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt);
    const auto translated_effect =
        (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt);

    auto translated_action = this->m_pddl_repositories.get_or_create_action(action.get_name(),
                                                                            action.get_original_arity(),
                                                                            translated_parameters,
                                                                            translated_conditions,
                                                                            translated_effect);

    return translated_action;
}

loki::Axiom RenameQuantifiedVariablesTranslator::translate_impl(const loki::AxiomImpl& axiom)
{
    initialize_num_quantifications(m_variables, m_num_quantifications);
    increment_num_quantifications(axiom.get_parameters(), m_num_quantifications);

    const auto translated_parameters = this->translate(axiom.get_parameters());
    const auto translated_conditions = this->translate(*axiom.get_condition());

    auto translated_axiom = this->m_pddl_repositories.get_or_create_axiom(axiom.get_derived_predicate_name(),
                                                                          translated_parameters,
                                                                          translated_conditions,
                                                                          axiom.get_num_parameters_to_ground_head());

    return translated_axiom;
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    increment_num_quantifications(condition.get_parameters(), m_num_quantifications);

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    auto translated_condition = this->m_pddl_repositories.get_or_create_condition(
        this->m_pddl_repositories.get_or_create_condition_exists(translated_parameters, translated_nested_condition));

    return translated_condition;
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    increment_num_quantifications(condition.get_parameters(), m_num_quantifications);

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    auto translated_condition = this->m_pddl_repositories.get_or_create_condition(
        this->m_pddl_repositories.get_or_create_condition_forall(translated_parameters, translated_nested_condition));

    return translated_condition;
}

loki::Effect RenameQuantifiedVariablesTranslator::translate_impl(const loki::EffectCompositeForallImpl& effect)
{
    increment_num_quantifications(effect.get_parameters(), m_num_quantifications);

    const auto translated_parameters = this->translate(effect.get_parameters());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    auto translated_effect = this->m_pddl_repositories.get_or_create_effect(
        this->m_pddl_repositories.get_or_create_effect_composite_forall(translated_parameters, translated_nested_effect));

    return translated_effect;
}

loki::Problem RenameQuantifiedVariablesTranslator::run_impl(const loki::ProblemImpl& problem)
{
    this->prepare(problem);

    initialize_num_quantifications(m_variables, m_num_quantifications);

    return this->translate(problem);
}

RenameQuantifiedVariablesTranslator::RenameQuantifiedVariablesTranslator(loki::PDDLRepositories& pddl_repositories) :
    BaseRecurseTranslator(pddl_repositories),
    m_num_quantifications(),
    m_renaming_enabled(true)
{
}
}
