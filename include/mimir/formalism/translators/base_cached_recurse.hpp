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

#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_

#include "mimir/formalism/translators/interface.hpp"

#include <deque>
#include <loki/loki.hpp>
#include <unordered_map>

namespace mimir
{

/**
 * Base implementation recursively calls translate and caches the results.
 */
template<typename Derived>
class BaseCachedRecurseTranslator : public ITranslator<BaseCachedRecurseTranslator<Derived>>
{
private:
    BaseCachedRecurseTranslator() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    std::unordered_map<loki::Requirements, loki::Requirements> m_translated_requirements;
    std::unordered_map<loki::Type, loki::Type> m_translated_types;
    std::unordered_map<loki::Object, loki::Object> m_translated_objects;
    std::unordered_map<loki::Variable, loki::Variable> m_translated_variables;
    std::unordered_map<loki::Term, loki::Term> m_translated_terms;
    std::unordered_map<loki::Parameter, loki::Parameter> m_translated_parameters;
    std::unordered_map<loki::Predicate, loki::Predicate> m_translated_predicates;
    std::unordered_map<loki::Atom, loki::Atom> m_translated_atoms;
    std::unordered_map<loki::Literal, loki::Literal> m_translated_literals;
    std::unordered_map<loki::NumericFluent, loki::NumericFluent> m_translated_numeric_fluents;
    std::unordered_map<loki::Condition, loki::Condition> m_translated_conditions;
    std::unordered_map<loki::Effect, loki::Effect> m_translated_effects;
    std::unordered_map<loki::FunctionExpression, loki::FunctionExpression> m_translated_function_expressions;
    std::unordered_map<loki::FunctionSkeleton, loki::FunctionSkeleton> m_translated_function_skeletons;
    std::unordered_map<loki::Function, loki::Function> m_translated_functions;
    std::unordered_map<loki::Action, loki::Action> m_translated_actions;
    std::unordered_map<loki::Axiom, loki::Axiom> m_translated_axioms;
    std::unordered_map<loki::Domain, loki::Domain> m_translated_domains;
    std::unordered_map<loki::OptimizationMetric, loki::OptimizationMetric> m_translated_optimization_metrics;
    std::unordered_map<loki::Problem, loki::Problem> m_translated_problems;

protected:
    loki::PDDLFactories& m_pddl_factories;

    explicit BaseCachedRecurseTranslator(loki::PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

protected:
    /* Implement ITranslator interface */
    friend class ITranslator<BaseCachedRecurseTranslator<Derived>>;

    /**
     * Collect information.
     * Default implementation recursively calls prepare.
     */
    void prepare_base(const loki::RequirementsImpl& requirements) { self().prepare_impl(requirements); }
    void prepare_base(const loki::TypeImpl& type) { self().prepare_impl(type); }
    void prepare_base(const loki::ObjectImpl& object) { self().prepare_impl(object); }
    void prepare_base(const loki::VariableImpl& variable) { self().prepare_impl(variable); }
    void prepare_base(const loki::TermObjectImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::TermVariableImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::TermImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::ParameterImpl& parameter) { self().prepare_impl(parameter); }
    void prepare_base(const loki::PredicateImpl& predicate) { self().prepare_impl(predicate); }
    void prepare_base(const loki::AtomImpl& atom) { self().prepare_impl(atom); }
    void prepare_base(const loki::LiteralImpl& literal) { self().prepare_impl(literal); }
    void prepare_base(const loki::NumericFluentImpl& numeric_fluent) { self().prepare_impl(numeric_fluent); }
    void prepare_base(const loki::ConditionLiteralImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionAndImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionOrImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionNotImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionImplyImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionExistsImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionForallImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::ConditionImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::EffectLiteralImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::EffectAndImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::EffectNumericImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::EffectConditionalForallImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::EffectConditionalWhenImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::EffectImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_base(const loki::FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_base(const loki::FunctionExpressionImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::FunctionSkeletonImpl& function_skeleton) { self().prepare_impl(function_skeleton); }
    void prepare_base(const loki::FunctionImpl& function) { self().prepare_impl(function); }
    void prepare_base(const loki::ActionImpl& action) { self().prepare_impl(action); }
    void prepare_base(const loki::AxiomImpl& axiom) { self().prepare_impl(axiom); }
    void prepare_base(const loki::DomainImpl& domain) { self().prepare_impl(domain); }
    void prepare_base(const loki::OptimizationMetricImpl& metric) { self().prepare_impl(metric); }
    void prepare_base(const loki::ProblemImpl& problem) { self().prepare_impl(problem); }

    void prepare_impl(const loki::RequirementsImpl& requirements) {}
    void prepare_impl(const loki::TypeImpl& type) { this->prepare(type.get_bases()); }
    void prepare_impl(const loki::ObjectImpl& object) { this->prepare(object.get_bases()); }
    void prepare_impl(const loki::VariableImpl& variable) {}
    void prepare_impl(const loki::TermObjectImpl& term) { this->prepare(*term.get_object()); }
    void prepare_impl(const loki::TermVariableImpl& term) { this->prepare(*term.get_variable()); }
    void prepare_impl(const loki::TermImpl& term)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, term);
    }
    void prepare_impl(const loki::ParameterImpl& parameter) { this->prepare(*parameter.get_variable()); }
    void prepare_impl(const loki::PredicateImpl& predicate) { this->prepare(predicate.get_parameters()); }
    void prepare_impl(const loki::AtomImpl& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_terms());
    }
    void prepare_impl(const loki::LiteralImpl& literal) { this->prepare(*literal.get_atom()); }
    void prepare_impl(const loki::NumericFluentImpl& numeric_fluent) { this->prepare(*numeric_fluent.get_function()); }
    void prepare_impl(const loki::ConditionLiteralImpl& condition) { this->prepare(*condition.get_literal()); }
    void prepare_impl(const loki::ConditionAndImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const loki::ConditionOrImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const loki::ConditionNotImpl& condition) { this->prepare(*condition.get_condition()); }
    void prepare_impl(const loki::ConditionImplyImpl& condition)
    {
        this->prepare(*condition.get_condition_left());
        this->prepare(*condition.get_condition_right());
    }
    void prepare_impl(const loki::ConditionExistsImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const loki::ConditionForallImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const loki::ConditionImpl& condition)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, condition);
    }
    void prepare_impl(const loki::EffectLiteralImpl& effect) { this->prepare(*effect.get_literal()); }
    void prepare_impl(const loki::EffectAndImpl& effect) { this->prepare(effect.get_effects()); }
    void prepare_impl(const loki::EffectNumericImpl& effect)
    {
        this->prepare(*effect.get_function());
        this->prepare(*effect.get_function_expression());
    }
    void prepare_impl(const loki::EffectConditionalForallImpl& effect)
    {
        this->prepare(effect.get_parameters());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const loki::EffectConditionalWhenImpl& effect)
    {
        this->prepare(*effect.get_condition());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const loki::EffectImpl& effect)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, effect);
    }
    void prepare_impl(const loki::FunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        this->prepare(*function_expression.get_left_function_expression());
        this->prepare(*function_expression.get_right_function_expression());
    }
    void prepare_impl(const loki::FunctionExpressionMultiOperatorImpl& function_expression) { this->prepare(function_expression.get_function_expressions()); }
    void prepare_impl(const loki::FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_impl(const loki::FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_impl(const loki::FunctionExpressionImpl& function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression);
    }
    void prepare_impl(const loki::FunctionSkeletonImpl& function_skeleton)
    {
        this->prepare(function_skeleton.get_parameters());
        this->prepare(*function_skeleton.get_type());
    }
    void prepare_impl(const loki::FunctionImpl& function)
    {
        this->prepare(*function.get_function_skeleton());
        this->prepare(function.get_terms());
    }
    void prepare_impl(const loki::ActionImpl& action)
    {
        this->prepare(action.get_parameters());
        if (action.get_condition().has_value())
        {
            this->prepare(*action.get_condition().value());
        }
        if (action.get_effect().has_value())
        {
            this->prepare(*action.get_effect().value());
        }
    }
    void prepare_impl(const loki::AxiomImpl& axiom)
    {
        this->prepare(axiom.get_parameters());
        this->prepare(*axiom.get_condition());
    }
    void prepare_impl(const loki::DomainImpl& domain)
    {
        this->prepare(*domain.get_requirements());
        this->prepare(domain.get_types());
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
        this->prepare(domain.get_axioms());
    }
    void prepare_impl(const loki::OptimizationMetricImpl& metric) { this->prepare(*metric.get_function_expression()); }
    void prepare_impl(const loki::ProblemImpl& problem)
    {
        this->prepare(*problem.get_domain());
        this->prepare(*problem.get_requirements());
        this->prepare(problem.get_objects());
        this->prepare(problem.get_derived_predicates());
        this->prepare(problem.get_initial_literals());
        this->prepare(problem.get_numeric_fluents());
        if (problem.get_goal_condition().has_value())
        {
            this->prepare(*problem.get_goal_condition().value());
        }
        if (problem.get_optimization_metric().has_value())
        {
            this->prepare(*problem.get_optimization_metric().value());
        }
        this->prepare(problem.get_axioms());
    }

    /**
     * Apply problem translation.
     * Default behavior reparses it into the pddl_factories.
     */

    /// @brief Retrieve or create cache entry of translation to avoid recomputations.
    template<typename Impl, typename TranslateFunc>
    auto cached_translate_impl(const Impl& impl, std::unordered_map<const Impl*, const Impl*>& cache, const TranslateFunc& translateFunc)
    {
        // Access from cache
        auto it = cache.find(&impl);
        if (it != cache.end())
        {
            return it->second;
        }

        // Translate
        auto translated = translateFunc(impl);

        // Insert into cache
        cache.emplace(&impl, translated);

        return translated;
    }
    loki::Requirements translate_base(const loki::RequirementsImpl& requirements)
    {
        return cached_translate_impl(requirements, m_translated_requirements, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Type translate_base(const loki::TypeImpl& type)
    {
        return cached_translate_impl(type, m_translated_types, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Object translate_base(const loki::ObjectImpl& object)
    {
        return cached_translate_impl(object, m_translated_objects, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Variable translate_base(const loki::VariableImpl& variable)
    {
        return cached_translate_impl(variable, m_translated_variables, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Term translate_base(const loki::TermObjectImpl& term) { return self().translate_impl(term); }
    loki::Term translate_base(const loki::TermVariableImpl& term) { return self().translate_impl(term); }
    loki::Term translate_base(const loki::TermImpl& term)
    {
        return cached_translate_impl(term, m_translated_terms, [this, &term](const auto& arg) { return this->self().translate_impl(term); });
    }
    loki::Parameter translate_base(const loki::ParameterImpl& parameter)
    {
        return cached_translate_impl(parameter, m_translated_parameters, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Predicate translate_base(const loki::PredicateImpl& predicate)
    {
        return cached_translate_impl(predicate, m_translated_predicates, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Atom translate_base(const loki::AtomImpl& atom)
    {
        return cached_translate_impl(atom, m_translated_atoms, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Literal translate_base(const loki::LiteralImpl& literal)
    {
        return cached_translate_impl(literal, m_translated_literals, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::NumericFluent translate_base(const loki::NumericFluentImpl& numeric_fluent)
    {
        return cached_translate_impl(numeric_fluent, m_translated_numeric_fluents, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Condition translate_base(const loki::ConditionLiteralImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionAndImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionOrImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionNotImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionImplyImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionExistsImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionForallImpl& condition) { return self().translate_impl(condition); }
    loki::Condition translate_base(const loki::ConditionImpl& condition)
    {
        return cached_translate_impl(condition, m_translated_conditions, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Effect translate_base(const loki::EffectLiteralImpl& effect) { return self().translate_impl(effect); }
    loki::Effect translate_base(const loki::EffectAndImpl& effect) { return self().translate_impl(effect); }
    loki::Effect translate_base(const loki::EffectNumericImpl& effect) { return self().translate_impl(effect); }
    loki::Effect translate_base(const loki::EffectConditionalForallImpl& effect) { return self().translate_impl(effect); }
    loki::Effect translate_base(const loki::EffectConditionalWhenImpl& effect) { return self().translate_impl(effect); }
    loki::Effect translate_base(const loki::EffectImpl& effect)
    {
        return cached_translate_impl(effect, m_translated_effects, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionNumberImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionMinusImpl& function_expression) { return self().translate_impl(function_expression); }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionFunctionImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::FunctionExpression translate_base(const loki::FunctionExpressionImpl& function_expression)
    {
        return cached_translate_impl(function_expression,
                                     m_translated_function_expressions,
                                     [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::FunctionSkeleton translate_base(const loki::FunctionSkeletonImpl& function_skeleton)
    {
        return cached_translate_impl(function_skeleton, m_translated_function_skeletons, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Function translate_base(const loki::FunctionImpl& function)
    {
        return cached_translate_impl(function, m_translated_functions, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Action translate_base(const loki::ActionImpl& action)
    {
        return cached_translate_impl(action, m_translated_actions, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Axiom translate_base(const loki::AxiomImpl& axiom)
    {
        return cached_translate_impl(axiom, m_translated_axioms, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Domain translate_base(const loki::DomainImpl& domain)
    {
        return cached_translate_impl(domain, m_translated_domains, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::OptimizationMetric translate_base(const loki::OptimizationMetricImpl& metric)
    {
        return cached_translate_impl(metric, m_translated_optimization_metrics, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }
    loki::Problem translate_base(const loki::ProblemImpl& problem)
    {
        return cached_translate_impl(problem, m_translated_problems, [this](const auto& arg) { return this->self().translate_impl(arg); });
    }

    loki::Requirements translate_impl(const loki::RequirementsImpl& requirements)
    {
        return this->m_pddl_factories.get_or_create_requirements(requirements.get_requirements());
    }
    loki::Type translate_impl(const loki::TypeImpl& type)
    {
        return this->m_pddl_factories.get_or_create_type(type.get_name(), this->translate(type.get_bases()));
    }
    loki::Object translate_impl(const loki::ObjectImpl& object)
    {
        return this->m_pddl_factories.get_or_create_object(object.get_name(), this->translate(object.get_bases()));
    }
    loki::Variable translate_impl(const loki::VariableImpl& variable) { return this->m_pddl_factories.get_or_create_variable(variable.get_name()); }
    loki::Term translate_impl(const loki::TermObjectImpl& term)
    {
        return this->m_pddl_factories.get_or_create_term_object(this->translate(*term.get_object()));
    }
    loki::Term translate_impl(const loki::TermVariableImpl& term)
    {
        return this->m_pddl_factories.get_or_create_term_variable(this->translate(*term.get_variable()));
    }
    loki::Term translate_impl(const loki::TermImpl& term)
    {
        return std::visit([this](auto&& arg) { return this->translate(arg); }, term);
    }
    loki::Parameter translate_impl(const loki::ParameterImpl& parameter)
    {
        return this->m_pddl_factories.get_or_create_parameter(this->translate(*parameter.get_variable()), this->translate(parameter.get_bases()));
    }
    loki::Predicate translate_impl(const loki::PredicateImpl& predicate)
    {
        return this->m_pddl_factories.get_or_create_predicate(predicate.get_name(), this->translate(predicate.get_parameters()));
    }
    loki::Atom translate_impl(const loki::AtomImpl& atom)
    {
        return this->m_pddl_factories.get_or_create_atom(this->translate(*atom.get_predicate()), this->translate(atom.get_terms()));
    }
    loki::Literal translate_impl(const loki::LiteralImpl& literal)
    {
        return this->m_pddl_factories.get_or_create_literal(literal.is_negated(), this->translate(*literal.get_atom()));
    }
    loki::NumericFluent translate_impl(const loki::NumericFluentImpl& numeric_fluent)
    {
        return this->m_pddl_factories.get_or_create_numeric_fluent(this->translate(*numeric_fluent.get_function()), numeric_fluent.get_number());
    }
    loki::Condition translate_impl(const loki::ConditionLiteralImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_literal(this->translate(*condition.get_literal()));
    }
    loki::Condition translate_impl(const loki::ConditionAndImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_and(this->translate(condition.get_conditions()));
    }
    loki::Condition translate_impl(const loki::ConditionOrImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_or(this->translate(condition.get_conditions()));
    }
    loki::Condition translate_impl(const loki::ConditionNotImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_not(this->translate(*condition.get_condition()));
    }
    loki::Condition translate_impl(const loki::ConditionImplyImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_imply(this->translate(*condition.get_condition_left()),
                                                                    this->translate(*condition.get_condition_right()));
    }
    loki::Condition translate_impl(const loki::ConditionExistsImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_exists(this->translate(condition.get_parameters()), this->translate(*condition.get_condition()));
    }
    loki::Condition translate_impl(const loki::ConditionForallImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_forall(this->translate(condition.get_parameters()), this->translate(*condition.get_condition()));
    }
    loki::Condition translate_impl(const loki::ConditionImpl& condition)
    {
        return std::visit([this](auto&& arg) { return this->translate(arg); }, condition);
    }
    loki::Effect translate_impl(const loki::EffectLiteralImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_literal(this->translate(*effect.get_literal()));
    }
    loki::Effect translate_impl(const loki::EffectAndImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_and(this->translate(effect.get_effects()));
    }
    loki::Effect translate_impl(const loki::EffectNumericImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_numeric(effect.get_assign_operator(),
                                                                   this->translate(*effect.get_function()),
                                                                   this->translate(*effect.get_function_expression()));
    }
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_forall(this->translate(effect.get_parameters()), this->translate(*effect.get_effect()));
    }
    loki::Effect translate_impl(const loki::EffectConditionalWhenImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_when(this->translate(*effect.get_condition()), this->translate(*effect.get_effect()));
    }
    loki::Effect translate_impl(const loki::EffectImpl& effect)
    {
        return std::visit([this](auto&& arg) { return this->translate(arg); }, effect);
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_number(function_expression.get_number());
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                                        this->translate(*function_expression.get_left_function_expression()),
                                                                                        this->translate(*function_expression.get_right_function_expression()));
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                       this->translate(function_expression.get_function_expressions()));
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_minus(this->translate(*function_expression.get_function_expression()));
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_function(this->translate(*function_expression.get_function()));
    }
    loki::FunctionExpression translate_impl(const loki::FunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->translate(arg); }, function_expression);
    }
    loki::FunctionSkeleton translate_impl(const loki::FunctionSkeletonImpl& function_skeleton)
    {
        return this->m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(),
                                                                      this->translate(function_skeleton.get_parameters()),
                                                                      this->translate(*function_skeleton.get_type()));
    }
    loki::Function translate_impl(const loki::FunctionImpl& function)
    {
        return this->m_pddl_factories.get_or_create_function(this->translate(*function.get_function_skeleton()), this->translate(function.get_terms()));
    }
    loki::Action translate_impl(const loki::ActionImpl& action)
    {
        return this->m_pddl_factories.get_or_create_action(
            action.get_name(),
            action.get_original_arity(),
            this->translate(action.get_parameters()),
            (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt),
            (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt));
    }
    loki::Axiom translate_impl(const loki::AxiomImpl& axiom)
    {
        return this->m_pddl_factories.get_or_create_axiom(axiom.get_derived_predicate_name(),
                                                          this->translate(axiom.get_parameters()),
                                                          this->translate(*axiom.get_condition()),
                                                          axiom.get_num_parameters_to_ground_head());
    }
    loki::Domain translate_impl(const loki::DomainImpl& domain)
    {
        return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                           this->translate(*domain.get_requirements()),
                                                           this->translate(domain.get_types()),
                                                           this->translate(domain.get_constants()),
                                                           this->translate(domain.get_predicates()),
                                                           this->translate(domain.get_functions()),
                                                           this->translate(domain.get_actions()),
                                                           this->translate(domain.get_axioms()));
    }
    loki::OptimizationMetric translate_impl(const loki::OptimizationMetricImpl& metric)
    {
        return this->m_pddl_factories.get_or_create_optimization_metric(metric.get_optimization_metric(), this->translate(*metric.get_function_expression()));
    }

    loki::Problem translate_impl(const loki::ProblemImpl& problem)
    {
        return this->m_pddl_factories.get_or_create_problem(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            this->translate(*problem.get_requirements()),
            this->translate(problem.get_objects()),
            this->translate(problem.get_derived_predicates()),
            this->translate(problem.get_initial_literals()),
            this->translate(problem.get_numeric_fluents()),
            (problem.get_goal_condition().has_value() ? std::optional<loki::Condition>(this->translate(*problem.get_goal_condition().value())) : std::nullopt),
            (problem.get_optimization_metric().has_value() ?
                 std::optional<loki::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                 std::nullopt),
            this->translate(problem.get_axioms()));
    }

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    loki::Problem run_base(const loki::ProblemImpl& problem) { return self().run_impl(problem); }

    loki::Problem run_impl(const loki::ProblemImpl& problem)
    {
        self().prepare(problem);
        return self().translate(problem);
    }
};
}

#endif