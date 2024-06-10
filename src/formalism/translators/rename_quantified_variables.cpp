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

RenameQuantifiedVariablesTranslator::PrepareScope::PrepareScope(RenameQuantifiedVariablesTranslator::PrepareScope* parent) :
    m_parent(parent),
    m_quantified_in_scope_or_child_scopes(),
    m_name_conflict_detected(false)
{
}

void RenameQuantifiedVariablesTranslator::PrepareScope::insert(const loki::Variable& variable)
{
    if (m_quantified_in_scope_or_child_scopes.count(variable))
    {
        m_name_conflict_detected = true;
    }

    m_quantified_in_scope_or_child_scopes.insert(variable);
}

void RenameQuantifiedVariablesTranslator::PrepareScope::on_conflict_detected() { m_name_conflict_detected = true; }

RenameQuantifiedVariablesTranslator::PrepareScope* RenameQuantifiedVariablesTranslator::PrepareScope::get_parent() { return m_parent; }

const std::unordered_set<loki::Variable>& RenameQuantifiedVariablesTranslator::PrepareScope::get_quantified_in_scope_or_child_scopes() const
{
    return m_quantified_in_scope_or_child_scopes;
}

bool RenameQuantifiedVariablesTranslator::PrepareScope::get_name_conflict_detected() const { return m_name_conflict_detected; }

const RenameQuantifiedVariablesTranslator::PrepareScope&
RenameQuantifiedVariablesTranslator::PrepareScopeStack::open_scope(const loki::ParameterList& parameters)
{
    m_scopes.empty() ? m_scopes.push_back(std::make_unique<RenameQuantifiedVariablesTranslator::PrepareScope>()) :
                       m_scopes.push_back(std::make_unique<RenameQuantifiedVariablesTranslator::PrepareScope>(m_scopes.back().get()));

    auto& scope = *m_scopes.back();

    for (const auto& parameter : parameters)
    {
        scope.insert(parameter->get_variable());
    }

    return scope;
}

void RenameQuantifiedVariablesTranslator::PrepareScopeStack::close_scope()
{
    if (m_scopes.empty())
    {
        throw std::logic_error("Tried to close scope on empty scope stack.");
    }

    auto& scope = *m_scopes.back();

    if (scope.get_parent())
    {
        for (const auto& variable : scope.get_quantified_in_scope_or_child_scopes())
        {
            scope.get_parent()->insert(variable);
        }
    }

    m_scopes.pop_back();
}

void RenameQuantifiedVariablesTranslator::PrepareScopeStack::close_scope_soft()
{
    auto& scope = *m_scopes.back();

    if (scope.get_name_conflict_detected() && scope.get_parent())
    {
        scope.get_parent()->on_conflict_detected();
    }

    m_scopes.pop_back();
}

const RenameQuantifiedVariablesTranslator::PrepareScope& RenameQuantifiedVariablesTranslator::PrepareScopeStack::top() const
{
    if (m_scopes.empty())
    {
        throw std::logic_error("Tried to access topmost scope of an empty scope stack.");
    }

    return *m_scopes.back();
}

void RenameQuantifiedVariablesTranslator::increment_num_quantifications(const loki::ParameterList& parameters)
{
    for (const auto& parameter : parameters)
    {
        ++m_num_quantifications.at(parameter->get_variable());
    }
}

void RenameQuantifiedVariablesTranslator::decrement_num_quantifications(const loki::ParameterList& parameters)
{
    for (const auto& parameter : parameters)
    {
        --m_num_quantifications.at(parameter->get_variable());
    }
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::VariableImpl& variable) { m_variables.insert(&variable); }

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::ConditionExistsImpl& condition)
{
    m_scopes.open_scope(condition.get_parameters());

    this->prepare(condition.get_parameters());
    this->prepare(*condition.get_condition());

    m_scopes.close_scope();
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::ConditionForallImpl& condition)
{
    m_scopes.open_scope(condition.get_parameters());

    this->prepare(condition.get_parameters());
    this->prepare(*condition.get_condition());

    m_scopes.close_scope();
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::EffectConditionalForallImpl& effect)
{
    m_scopes.open_scope(effect.get_parameters());

    this->prepare(effect.get_parameters());
    this->prepare(*effect.get_effect());

    m_scopes.close_scope();
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::ActionImpl& action)
{
    m_scopes.open_scope(action.get_parameters());

    this->prepare(action.get_parameters());
    if (action.get_condition().has_value())
    {
        this->prepare(*action.get_condition().value());
    }
    if (action.get_effect().has_value())
    {
        this->prepare(*action.get_effect().value());
    }

    m_scopes.close_scope_soft();
}

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::AxiomImpl& axiom)
{
    m_scopes.open_scope(axiom.get_parameters());

    this->prepare(axiom.get_parameters());
    this->prepare(*axiom.get_condition());

    m_scopes.close_scope_soft();
}

loki::Variable RenameQuantifiedVariablesTranslator::translate_impl(const loki::VariableImpl& variable)
{
    return (m_renaming_enabled) ? m_pddl_factories.get_or_create_variable(variable.get_name() + "_" + std::to_string(variable.get_identifier()) + "_"
                                                                          + std::to_string(m_num_quantifications.at(&variable))) :
                                  m_pddl_factories.get_or_create_variable(variable.get_name());
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
    increment_num_quantifications(action.get_parameters());

    const auto translated_parameters = this->translate(action.get_parameters());
    const auto translated_conditions =
        (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt);
    const auto translated_effect =
        (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt);

    auto translated_action = this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                                         action.get_original_arity(),
                                                                         translated_parameters,
                                                                         translated_conditions,
                                                                         translated_effect);

    decrement_num_quantifications(action.get_parameters());

    return translated_action;
}

loki::Axiom RenameQuantifiedVariablesTranslator::translate_impl(const loki::AxiomImpl& axiom)
{
    increment_num_quantifications(axiom.get_parameters());

    const auto translated_parameters = this->translate(axiom.get_parameters());
    const auto translated_conditions = this->translate(*axiom.get_condition());

    auto translated_axiom = this->m_pddl_factories.get_or_create_axiom(axiom.get_derived_predicate_name(),
                                                                       translated_parameters,
                                                                       translated_conditions,
                                                                       axiom.get_num_parameters_to_ground_head());

    decrement_num_quantifications(axiom.get_parameters());

    return translated_axiom;
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    increment_num_quantifications(condition.get_parameters());

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    auto translated_condition = this->m_pddl_factories.get_or_create_condition_exists(translated_parameters, translated_nested_condition);

    decrement_num_quantifications(condition.get_parameters());

    return translated_condition;
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    increment_num_quantifications(condition.get_parameters());

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    auto translated_condition = this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, translated_nested_condition);

    decrement_num_quantifications(condition.get_parameters());

    return translated_condition;
}

loki::Effect RenameQuantifiedVariablesTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    increment_num_quantifications(effect.get_parameters());

    const auto translated_parameters = this->translate(effect.get_parameters());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    auto translated_effect = this->m_pddl_factories.get_or_create_effect_conditional_forall(translated_parameters, translated_nested_effect);

    decrement_num_quantifications(effect.get_parameters());

    return translated_effect;
}

loki::Problem RenameQuantifiedVariablesTranslator::run_impl(const loki::ProblemImpl& problem)
{
    // Open a scope that holds the result
    m_scopes.open_scope();

    this->prepare(problem);

    // Only run translate to rename if there was a conflict detected.
    if (m_scopes.top().get_name_conflict_detected())
    {
        // Initialize renaming info
        for (const auto& variable : m_variables)
        {
            m_num_quantifications.emplace(variable, 0);
        }

        return this->translate(problem);
    }

    return &problem;
}

RenameQuantifiedVariablesTranslator::RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories) :
    BaseRecurseTranslator(pddl_factories),
    m_num_quantifications(),
    m_renaming_enabled(true)
{
}
}
