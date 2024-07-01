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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_BASE_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/pddl.hpp"
#include "mimir/formalism/transformers/interface.hpp"

#include <deque>
#include <loki/loki.hpp>
#include <unordered_map>

namespace mimir
{

/**
 * Base implementation recursively calls prepare, followed by recursively calls transform and caches the results.
 */
template<typename Derived_>
class BaseTransformer : public ITransformer<BaseTransformer<Derived_>>
{
private:
    BaseTransformer() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    PDDLFactories& m_pddl_factories;

    explicit BaseTransformer(PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

protected:
    /* Implement ITranslator interface */
    friend class ITransformer<BaseTransformer<Derived_>>;

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    template<typename Container>
    void prepare_base(const Container& input)
    {
        self().prepare_impl(input);
    }
    void prepare_base(const RequirementsImpl& requirements) { self().prepare_impl(requirements); }
    void prepare_base(const ObjectImpl& object) { self().prepare_impl(object); }
    void prepare_base(const VariableImpl& variable) { self().prepare_impl(variable); }
    void prepare_base(const TermObjectImpl& term) { self().prepare_impl(term); }
    void prepare_base(const TermVariableImpl& term) { self().prepare_impl(term); }
    void prepare_base(const TermImpl& term) { self().prepare_impl(term); }
    template<PredicateCategory P>
    void prepare_base(const PredicateImpl<P>& predicate)
    {
        self().prepare_impl(predicate);
    }
    template<PredicateCategory P>
    void prepare_base(const AtomImpl<P>& atom)
    {
        self().prepare_impl(atom);
    }
    template<PredicateCategory P>
    void prepare_base(const GroundAtomImpl<P>& atom)
    {
        self().prepare_impl(atom);
    }
    template<PredicateCategory P>
    void prepare_base(const LiteralImpl<P>& literal)
    {
        self().prepare_impl(literal);
    }
    template<PredicateCategory P>
    void prepare_base(const GroundLiteralImpl<P>& literal)
    {
        self().prepare_impl(literal);
    }
    void prepare_base(const NumericFluentImpl& numeric_fluent) { self().prepare_impl(numeric_fluent); }
    void prepare_base(const EffectSimpleImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectConditionalImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectUniversalImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_base(const FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_base(const FunctionExpressionImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionNumberImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionBinaryOperatorImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionMultiOperatorImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionMinusImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionFunctionImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const GroundFunctionExpressionImpl& function_expression) { return self().prepare_impl(function_expression); }
    void prepare_base(const FunctionSkeletonImpl& function_skeleton) { self().prepare_impl(function_skeleton); }
    void prepare_base(const FunctionImpl& function) { self().prepare_impl(function); }
    void prepare_base(const GroundFunctionImpl& function) { return self().prepare_impl(function); }
    void prepare_base(const ActionImpl& action) { self().prepare_impl(action); }
    void prepare_base(const AxiomImpl& axiom) { self().prepare_impl(axiom); }
    void prepare_base(const DomainImpl& domain) { self().prepare_impl(domain); }
    void prepare_base(const OptimizationMetricImpl& metric) { self().prepare_impl(metric); }
    void prepare_base(const ProblemImpl& problem) { self().prepare_impl(problem); }

    template<typename Container>
    void prepare_impl(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare(*arg); });
    }
    void prepare_impl(const RequirementsImpl& requirements) {}
    void prepare_impl(const ObjectImpl& object) {}
    void prepare_impl(const VariableImpl& variable) {}
    void prepare_impl(const TermObjectImpl& term) { this->prepare(*term.get_object()); }
    void prepare_impl(const TermVariableImpl& term) { this->prepare(*term.get_variable()); }
    void prepare_impl(const TermImpl& term)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, term);
    }
    template<PredicateCategory P>
    void prepare_impl(const PredicateImpl<P>& predicate)
    {
        this->prepare(predicate.get_parameters());
    }
    template<PredicateCategory P>
    void prepare_impl(const AtomImpl<P>& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_terms());
    }
    template<PredicateCategory P>
    void prepare_impl(const GroundAtomImpl<P>& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_objects());
    }
    template<PredicateCategory P>
    void prepare_impl(const LiteralImpl<P>& literal)
    {
        this->prepare(*literal.get_atom());
    }
    template<PredicateCategory P>
    void prepare_impl(const GroundLiteralImpl<P>& literal)
    {
        this->prepare(*literal.get_atom());
    }
    void prepare_impl(const NumericFluentImpl& numeric_fluent) { this->prepare(*numeric_fluent.get_function()); }
    void prepare_impl(const EffectSimpleImpl& effect) { this->prepare(*effect.get_effect()); }
    void prepare_impl(const EffectConditionalImpl& effect)
    {
        this->prepare(effect.get_conditions<Static>());
        this->prepare(effect.get_conditions<Fluent>());
        this->prepare(effect.get_conditions<Derived>());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const EffectUniversalImpl& effect)
    {
        this->prepare(effect.get_parameters());
        this->prepare(effect.get_conditions<Static>());
        this->prepare(effect.get_conditions<Fluent>());
        this->prepare(effect.get_conditions<Derived>());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const FunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        this->prepare(*function_expression.get_left_function_expression());
        this->prepare(*function_expression.get_right_function_expression());
    }
    void prepare_impl(const FunctionExpressionMultiOperatorImpl& function_expression) { this->prepare(function_expression.get_function_expressions()); }
    void prepare_impl(const FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_impl(const FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_impl(const FunctionExpressionImpl& function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression);
    }
    void prepare_impl(const GroundFunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
    {
        this->prepare(*function_expression.get_left_function_expression());
        this->prepare(*function_expression.get_right_function_expression());
    }
    void prepare_impl(const GroundFunctionExpressionMultiOperatorImpl& function_expression) { this->prepare(function_expression.get_function_expressions()); }
    void prepare_impl(const GroundFunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_impl(const GroundFunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_impl(const GroundFunctionExpressionImpl& function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression);
    }
    void prepare_impl(const FunctionSkeletonImpl& function_skeleton) { this->prepare(function_skeleton.get_parameters()); }
    void prepare_impl(const FunctionImpl& function)
    {
        this->prepare(*function.get_function_skeleton());
        this->prepare(function.get_terms());
    }
    void prepare_impl(const GroundFunctionImpl& function)
    {
        this->prepare(*function.get_function_skeleton());
        this->prepare(function.get_objects());
    }
    void prepare_impl(const ActionImpl& action)
    {
        this->prepare(action.get_parameters());
        this->prepare(action.get_conditions<Static>());
        this->prepare(action.get_conditions<Fluent>());
        this->prepare(action.get_conditions<Derived>());
        this->prepare(action.get_simple_effects());
        this->prepare(action.get_conditional_effects());
        this->prepare(action.get_universal_effects());
        this->prepare(*action.get_function_expression());
    }
    void prepare_impl(const AxiomImpl& axiom)
    {
        this->prepare(axiom.get_parameters());
        this->prepare(axiom.get_conditions<Static>());
        this->prepare(axiom.get_conditions<Fluent>());
        this->prepare(axiom.get_conditions<Derived>());
        this->prepare(*axiom.get_literal());
    }
    void prepare_impl(const DomainImpl& domain)
    {
        this->prepare(*domain.get_requirements());
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates<Static>());
        this->prepare(domain.get_predicates<Fluent>());
        this->prepare(domain.get_predicates<Derived>());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
        this->prepare(domain.get_axioms());
    }
    void prepare_impl(const OptimizationMetricImpl& metric) { this->prepare(*metric.get_function_expression()); }
    void prepare_impl(const ProblemImpl& problem)
    {
        this->prepare(*problem.get_domain());
        this->prepare(*problem.get_requirements());
        this->prepare(problem.get_objects());
        this->prepare(problem.get_derived_predicates());
        this->prepare(problem.get_static_initial_literals());
        this->prepare(problem.get_fluent_initial_literals());
        this->prepare(problem.get_numeric_fluents());
        this->prepare(problem.get_goal_condition<Static>());
        this->prepare(problem.get_goal_condition<Fluent>());
        this->prepare(problem.get_goal_condition<Derived>());
        if (problem.get_optimization_metric().has_value())
        {
            this->prepare(*problem.get_optimization_metric().value());
        }
        this->prepare(problem.get_axioms());
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    template<typename Container>
    auto transform_base(const Container& input)
    {
        return this->self().transform_impl(input);
    }
    Requirements transform_base(const RequirementsImpl& requirements) { return this->self().transform_impl(requirements); }
    Object transform_base(const ObjectImpl& object) { return this->self().transform_impl(object); }
    Variable transform_base(const VariableImpl& variable) { return this->self().transform_impl(variable); }
    Term transform_base(const TermObjectImpl& term) { return this->self().transform_impl(term); }
    Term transform_base(const TermVariableImpl& term) { return this->self().transform_impl(term); }
    Term transform_base(const TermImpl& term)
    {
        return std::visit([this](auto&& arg) { return this->transform_impl(arg); }, term);
    }
    template<PredicateCategory P>
    Predicate<P> transform_base(const PredicateImpl<P>& predicate)
    {
        return this->self().transform_impl(predicate);
    }
    template<PredicateCategory P>
    Atom<P> transform_base(const AtomImpl<P>& atom)
    {
        return this->self().transform_impl(atom);
    }
    template<PredicateCategory P>
    GroundAtom<P> transform_base(const GroundAtomImpl<P>& atom)
    {
        return this->self().transform_impl(atom);
    }
    template<PredicateCategory P>
    Literal<P> transform_base(const LiteralImpl<P>& literal)
    {
        return this->self().transform_impl(literal);
    }
    template<PredicateCategory P>
    GroundLiteral<P> transform_base(const GroundLiteralImpl<P>& literal)
    {
        return this->self().transform_impl(literal);
    }
    NumericFluent transform_base(const NumericFluentImpl& numeric_fluent) { return this->self().transform_impl(numeric_fluent); }
    EffectSimple transform_base(const EffectSimpleImpl& effect) { return this->self().transform_impl(effect); }
    EffectConditional transform_base(const EffectConditionalImpl& effect) { return this->self().transform_impl(effect); }
    EffectUniversal transform_base(const EffectUniversalImpl& effect) { return this->self().transform_impl(effect); }
    FunctionExpression transform_base(const FunctionExpressionNumberImpl& function_expression) { return self().transform_impl(function_expression); }
    FunctionExpression transform_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().transform_impl(function_expression); }
    FunctionExpression transform_base(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().transform_impl(function_expression); }
    FunctionExpression transform_base(const FunctionExpressionMinusImpl& function_expression) { return self().transform_impl(function_expression); }
    FunctionExpression transform_base(const FunctionExpressionFunctionImpl& function_expression) { return self().transform_impl(function_expression); }
    FunctionExpression transform_base(const FunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->transform_impl(arg); }, function_expression);
    }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionNumberImpl& function_expression)
    {
        return self().transform_impl(function_expression);
    }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().transform_impl(function_expression);
    }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().transform_impl(function_expression);
    }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionMinusImpl& function_expression) { return self().transform_impl(function_expression); }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionFunctionImpl& function_expression)
    {
        return self().transform_impl(function_expression);
    }
    GroundFunctionExpression transform_base(const GroundFunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->transform_impl(arg); }, function_expression);
    }
    FunctionSkeleton transform_base(const FunctionSkeletonImpl& function_skeleton) { return this->self().transform_impl(function_skeleton); }
    Function transform_base(const FunctionImpl& function) { return this->self().transform_impl(function); }
    GroundFunction transform_base(const GroundFunctionImpl& function) { return this->self().transform_impl(function); }
    Action transform_base(const ActionImpl& action) { return this->self().transform_impl(action); }
    Axiom transform_base(const AxiomImpl& axiom) { return this->self().transform_impl(axiom); }
    Domain transform_base(const DomainImpl& domain) { return this->self().transform_impl(domain); }
    OptimizationMetric transform_base(const OptimizationMetricImpl& metric) { return this->self().transform_impl(metric); }
    Problem transform_base(const ProblemImpl& problem) { return this->self().transform_impl(problem); }

    template<typename Container>
    auto transform_impl(const Container& input)
    {
        Container output;
        output.reserve(input.size());
        std::transform(std::begin(input), std::end(input), std::back_inserter(output), [this](auto&& arg) { return this->transform(*arg); });
        return output;
    }
    Requirements transform_impl(const RequirementsImpl& requirements)
    {
        return this->m_pddl_factories.get_or_create_requirements(requirements.get_requirements());
    }
    Object transform_impl(const ObjectImpl& object) { return this->m_pddl_factories.get_or_create_object(object.get_name()); }
    Variable transform_impl(const VariableImpl& variable)
    {
        return this->m_pddl_factories.get_or_create_variable(variable.get_name(), variable.get_parameter_index());
    }
    Term transform_impl(const TermObjectImpl& term) { return this->m_pddl_factories.get_or_create_term_object(this->transform(*term.get_object())); }
    Term transform_impl(const TermVariableImpl& term) { return this->m_pddl_factories.get_or_create_term_variable(this->transform(*term.get_variable())); }
    Term transform_impl(const TermImpl& term)
    {
        return std::visit([this](auto&& arg) { return this->transform(arg); }, term);
    }
    Predicate<Static> transform_impl(const PredicateImpl<Static>& predicate)
    {
        return this->m_pddl_factories.template get_or_create_predicate<Static>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    Predicate<Fluent> transform_impl(const PredicateImpl<Fluent>& predicate)
    {
        return this->m_pddl_factories.template get_or_create_predicate<Fluent>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    Predicate<Derived> transform_impl(const PredicateImpl<Derived>& predicate)
    {
        return this->m_pddl_factories.template get_or_create_predicate<Derived>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    template<PredicateCategory P>
    Atom<P> transform_impl(const AtomImpl<P>& atom)
    {
        return this->m_pddl_factories.get_or_create_atom(this->transform(*atom.get_predicate()), this->transform(atom.get_terms()));
    }
    template<PredicateCategory P>
    GroundAtom<P> transform_impl(const GroundAtomImpl<P>& atom)
    {
        return this->m_pddl_factories.get_or_create_ground_atom(this->transform(*atom.get_predicate()), this->transform(atom.get_objects()));
    }
    template<PredicateCategory P>
    Literal<P> transform_impl(const LiteralImpl<P>& literal)
    {
        return this->m_pddl_factories.get_or_create_literal(literal.is_negated(), this->transform(*literal.get_atom()));
    }
    template<PredicateCategory P>
    GroundLiteral<P> transform_impl(const GroundLiteralImpl<P>& literal)
    {
        return this->m_pddl_factories.get_or_create_ground_literal(literal.is_negated(), this->transform(*literal.get_atom()));
    }
    NumericFluent transform_impl(const NumericFluentImpl& numeric_fluent)
    {
        return this->m_pddl_factories.get_or_create_numeric_fluent(this->transform(*numeric_fluent.get_function()), numeric_fluent.get_number());
    }
    EffectSimple transform_impl(const EffectSimpleImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_simple_effect(this->transform(*effect.get_effect()));
    }
    EffectConditional transform_impl(const EffectConditionalImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_conditional_effect(this->transform(effect.get_conditions<Static>()),
                                                                       this->transform(effect.get_conditions<Fluent>()),
                                                                       this->transform(effect.get_conditions<Derived>()),
                                                                       this->transform(*effect.get_effect()));
    }
    EffectUniversal transform_impl(const EffectUniversalImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_universal_effect(this->transform(effect.get_parameters()),
                                                                     this->transform(effect.get_conditions<Static>()),
                                                                     this->transform(effect.get_conditions<Fluent>()),
                                                                     this->transform(effect.get_conditions<Derived>()),
                                                                     this->transform(*effect.get_effect()));
    }
    FunctionExpression transform_impl(const FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_number(function_expression.get_number());
    }
    FunctionExpression transform_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                                        this->transform(*function_expression.get_left_function_expression()),
                                                                                        this->transform(*function_expression.get_right_function_expression()));
    }
    FunctionExpression transform_impl(const FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                       this->transform(function_expression.get_function_expressions()));
    }
    FunctionExpression transform_impl(const FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_minus(this->transform(*function_expression.get_function_expression()));
    }
    FunctionExpression transform_impl(const FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_function(this->transform(*function_expression.get_function()));
    }
    FunctionExpression transform_impl(const FunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->transform(arg); }, function_expression);
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_ground_function_expression_number(function_expression.get_number());
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_ground_function_expression_binary_operator(
            function_expression.get_binary_operator(),
            this->transform(*function_expression.get_left_function_expression()),
            this->transform(*function_expression.get_right_function_expression()));
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_ground_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                              this->transform(function_expression.get_function_expressions()));
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_ground_function_expression_minus(this->transform(*function_expression.get_function_expression()));
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_ground_function_expression_function(this->transform(*function_expression.get_function()));
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->transform(arg); }, function_expression);
    }
    FunctionSkeleton transform_impl(const FunctionSkeletonImpl& function_skeleton)
    {
        return this->m_pddl_factories.get_or_create_function_skeleton(function_skeleton.get_name(), this->transform(function_skeleton.get_parameters()));
    }
    Function transform_impl(const FunctionImpl& function)
    {
        return this->m_pddl_factories.get_or_create_function(this->transform(*function.get_function_skeleton()), this->transform(function.get_terms()));
    }
    GroundFunction transform_impl(const GroundFunctionImpl& function)
    {
        return this->m_pddl_factories.get_or_create_ground_function(this->transform(*function.get_function_skeleton()),
                                                                    this->transform(function.get_objects()));
    }
    Action transform_impl(const ActionImpl& action)
    {
        return this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                           action.get_original_arity(),
                                                           this->transform(action.get_parameters()),
                                                           this->transform(action.get_conditions<Static>()),
                                                           this->transform(action.get_conditions<Fluent>()),
                                                           this->transform(action.get_conditions<Derived>()),
                                                           this->transform(action.get_simple_effects()),
                                                           this->transform(action.get_conditional_effects()),
                                                           this->transform(action.get_universal_effects()),
                                                           this->transform(*action.get_function_expression()));
    }
    Axiom transform_impl(const AxiomImpl& axiom)
    {
        return this->m_pddl_factories.get_or_create_axiom(this->transform(axiom.get_parameters()),
                                                          this->transform(*axiom.get_literal()),
                                                          this->transform(axiom.get_conditions<Static>()),
                                                          this->transform(axiom.get_conditions<Fluent>()),
                                                          this->transform(axiom.get_conditions<Derived>()));
    }
    Domain transform_impl(const DomainImpl& domain)
    {
        return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                           this->transform(*domain.get_requirements()),
                                                           this->transform(domain.get_constants()),
                                                           this->transform(domain.get_predicates<Static>()),
                                                           this->transform(domain.get_predicates<Fluent>()),
                                                           this->transform(domain.get_predicates<Derived>()),
                                                           this->transform(domain.get_functions()),
                                                           this->transform(domain.get_actions()),
                                                           this->transform(domain.get_axioms()));
    }
    OptimizationMetric transform_impl(const OptimizationMetricImpl& metric)
    {
        return this->m_pddl_factories.get_or_create_optimization_metric(metric.get_optimization_metric(), this->transform(*metric.get_function_expression()));
    }

    Problem transform_impl(const ProblemImpl& problem)
    {
        return this->m_pddl_factories.get_or_create_problem(
            this->transform(*problem.get_domain()),
            problem.get_name(),
            this->transform(*problem.get_requirements()),
            this->transform(problem.get_objects()),
            this->transform(problem.get_derived_predicates()),
            this->transform(problem.get_problem_and_domain_derived_predicates()),
            this->transform(problem.get_static_initial_literals()),
            this->transform(problem.get_fluent_initial_literals()),
            this->transform(problem.get_numeric_fluents()),
            this->transform(problem.get_goal_condition<Static>()),
            this->transform(problem.get_goal_condition<Fluent>()),
            this->transform(problem.get_goal_condition<Derived>()),
            (problem.get_optimization_metric().has_value() ? std::optional<OptimizationMetric>(this->transform(*problem.get_optimization_metric().value())) :
                                                             std::nullopt),
            this->transform(problem.get_axioms()));
    }

    /// @brief Recursively apply preparation followed by transformation.
    ///        Default behavior runs prepare and transform and returns its results.
    Problem run_base(const ProblemImpl& problem) { return self().run_impl(problem); }

    Problem run_impl(const ProblemImpl& problem) { return self().transform(problem); }
};
}

#endif