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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_BASE_RECURSE_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_BASE_RECURSE_HPP_

#include "mimir/formalism/repositories.hpp"
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
class BaseRecurseTransformer : public ITransformer<BaseRecurseTransformer<Derived_>>
{
private:
    BaseRecurseTransformer() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    PDDLRepositories& m_pddl_repositories;

    explicit BaseRecurseTransformer(PDDLRepositories& pddl_repositories) : m_pddl_repositories(pddl_repositories) {}

protected:
    /* Implement ITranslator interface */
    friend class ITransformer<BaseRecurseTransformer<Derived_>>;

    template<std::ranges::forward_range Range>
    void prepare_base(const Range& input)
    {
        self().prepare_impl(input);
    }
    template<std::ranges::forward_range Range>
    void prepare_impl(const Range& input)
    {
        std::ranges::for_each(input, [this](auto&& arg) { this->prepare(*arg); });
    }

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    template<typename T>
    void prepare_base(const T& element)
    {
        self().prepare_impl(element);
    }

    template<typename Container>
    void prepare_impl(const Container& input)
    {
        std::for_each(std::begin(input), std::end(input), [this](auto&& arg) { this->prepare(*arg); });
    }
    void prepare_impl(const RequirementsImpl& requirements) {}
    void prepare_impl(const ObjectImpl& object) {}
    void prepare_impl(const VariableImpl& variable) {}
    void prepare_impl(const TermImpl& term)
    {
        std::visit([this](auto&& arg) { return this->prepare(*arg); }, term.get_variant());
    }
    template<PredicateTag P>
    void prepare_impl(const PredicateImpl<P>& predicate)
    {
        this->prepare(predicate.get_parameters());
    }
    template<PredicateTag P>
    void prepare_impl(const AtomImpl<P>& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_terms());
    }
    template<PredicateTag P>
    void prepare_impl(const GroundAtomImpl<P>& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_objects());
    }
    template<PredicateTag P>
    void prepare_impl(const LiteralImpl<P>& literal)
    {
        this->prepare(*literal.get_atom());
    }
    template<PredicateTag P>
    void prepare_impl(const GroundLiteralImpl<P>& literal)
    {
        this->prepare(*literal.get_atom());
    }
    void prepare_impl(const NumericFluentImpl& numeric_fluent) { this->prepare(*numeric_fluent.get_function()); }
    void prepare_impl(const EffectStripsImpl& effect)
    {
        this->prepare(effect.get_effects());
        this->prepare(*effect.get_function_expression());
    }
    void prepare_impl(const EffectConditionalImpl& effect)
    {
        this->prepare(effect.get_parameters());
        this->prepare(effect.get_conditions<Static>());
        this->prepare(effect.get_conditions<Fluent>());
        this->prepare(effect.get_conditions<Derived>());
        this->prepare(effect.get_effects());
        this->prepare(*effect.get_function_expression());
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
        this->prepare(action.get_strips_effect());
        this->prepare(action.get_conditional_effects());
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

    /// @brief Translate a container of elements into a container of elements.
    template<IsBackInsertibleRange Range>
    auto transform_base(const Range& input)
    {
        return self().transform_impl(input);
    }
    /// @brief Translate a container of elements into a container of elements.
    template<IsBackInsertibleRange Range>
    auto transform_impl(const Range& input)
    {
        std::remove_cvref_t<Range> output;

        if constexpr (requires { output.reserve(std::ranges::size(input)); })
        {
            output.reserve(std::ranges::size(input));
        }

        std::ranges::transform(input, std::back_inserter(output), [this](auto&& arg) { return this->transform(*arg); });

        return output;
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_repositories.
    template<typename T>
    auto transform_base(const T& element)
    {
        return self().transform_impl(element);
    }

    Requirements transform_impl(const RequirementsImpl& requirements)
    {
        return this->m_pddl_repositories.get_or_create_requirements(requirements.get_requirements());
    }
    Object transform_impl(const ObjectImpl& object) { return this->m_pddl_repositories.get_or_create_object(object.get_name()); }
    Variable transform_impl(const VariableImpl& variable)
    {
        return this->m_pddl_repositories.get_or_create_variable(variable.get_name(), variable.get_parameter_index());
    }
    Term transform_impl(const TermImpl& term)
    {
        return std::visit(
            [this](auto&& arg) -> Term
            {
                using ArgType = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<ArgType, Variable>)
                {
                    return this->m_pddl_repositories.get_or_create_term(this->transform(*arg));
                }
                else if constexpr (std::is_same_v<ArgType, Object>)
                {
                    return this->m_pddl_repositories.get_or_create_term(this->transform(*arg));
                }
                else
                {
                    static_assert(dependent_false<ArgType>::value, "Missing implementation for ArgType.");
                }
            },
            term.get_variant());
    }
    Predicate<Static> transform_impl(const PredicateImpl<Static>& predicate)
    {
        return this->m_pddl_repositories.template get_or_create_predicate<Static>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    Predicate<Fluent> transform_impl(const PredicateImpl<Fluent>& predicate)
    {
        return this->m_pddl_repositories.template get_or_create_predicate<Fluent>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    Predicate<Derived> transform_impl(const PredicateImpl<Derived>& predicate)
    {
        return this->m_pddl_repositories.template get_or_create_predicate<Derived>(predicate.get_name(), this->transform(predicate.get_parameters()));
    }
    template<PredicateTag P>
    Atom<P> transform_impl(const AtomImpl<P>& atom)
    {
        return this->m_pddl_repositories.get_or_create_atom(this->transform(*atom.get_predicate()), this->transform(atom.get_terms()));
    }
    template<PredicateTag P>
    GroundAtom<P> transform_impl(const GroundAtomImpl<P>& atom)
    {
        return this->m_pddl_repositories.get_or_create_ground_atom(this->transform(*atom.get_predicate()), this->transform(atom.get_objects()));
    }
    template<PredicateTag P>
    Literal<P> transform_impl(const LiteralImpl<P>& literal)
    {
        return this->m_pddl_repositories.get_or_create_literal(literal.is_negated(), this->transform(*literal.get_atom()));
    }
    template<PredicateTag P>
    GroundLiteral<P> transform_impl(const GroundLiteralImpl<P>& literal)
    {
        return this->m_pddl_repositories.get_or_create_ground_literal(literal.is_negated(), this->transform(*literal.get_atom()));
    }
    NumericFluent transform_impl(const NumericFluentImpl& numeric_fluent)
    {
        return this->m_pddl_repositories.get_or_create_numeric_fluent(this->transform(*numeric_fluent.get_function()), numeric_fluent.get_number());
    }
    EffectStrips transform_impl(const EffectStripsImpl& effect)
    {
        return this->m_pddl_repositories.get_or_create_strips_effect(this->transform(effect.get_effects()), this->transform(*effect.get_function_expression()));
    }
    EffectConditional transform_impl(const EffectConditionalImpl& effect)
    {
        return this->m_pddl_repositories.get_or_create_conditional_effect(this->transform(effect.get_parameters()),
                                                                          this->transform(effect.get_conditions<Static>()),
                                                                          this->transform(effect.get_conditions<Fluent>()),
                                                                          this->transform(effect.get_conditions<Derived>()),
                                                                          this->transform(effect.get_effects()),
                                                                          this->transform(*effect.get_function_expression()));
    }
    FunctionExpressionNumber transform_impl(const FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_number(function_expression.get_number());
    }
    FunctionExpressionBinaryOperator transform_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_binary_operator(
            function_expression.get_binary_operator(),
            this->transform(*function_expression.get_left_function_expression()),
            this->transform(*function_expression.get_right_function_expression()));
    }
    FunctionExpressionMultiOperator transform_impl(const FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                          this->transform(function_expression.get_function_expressions()));
    }
    FunctionExpressionMinus transform_impl(const FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_minus(this->transform(*function_expression.get_function_expression()));
    }
    FunctionExpressionFunction transform_impl(const FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_function(this->transform(*function_expression.get_function()));
    }
    FunctionExpression transform_impl(const FunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->m_pddl_repositories.get_or_create_function_expression(this->transform(*arg)); },
                          function_expression.get_variant());
    }
    GroundFunctionExpressionNumber transform_impl(const GroundFunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_number(function_expression.get_number());
    }
    GroundFunctionExpressionBinaryOperator transform_impl(const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_binary_operator(
            function_expression.get_binary_operator(),
            this->transform(*function_expression.get_left_function_expression()),
            this->transform(*function_expression.get_right_function_expression()));
    }
    GroundFunctionExpressionMultiOperator transform_impl(const GroundFunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_multi_operator(
            function_expression.get_multi_operator(),
            this->transform(function_expression.get_function_expressions()));
    }
    GroundFunctionExpressionMinus transform_impl(const GroundFunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_minus(this->transform(*function_expression.get_function_expression()));
    }
    GroundFunctionExpressionFunction transform_impl(const GroundFunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_function(this->transform(*function_expression.get_function()));
    }
    GroundFunctionExpression transform_impl(const GroundFunctionExpressionImpl& function_expression)
    {
        return std::visit([this](auto&& arg) { return this->m_pddl_repositories.get_or_create_ground_function_expression(this->transform(*arg)); },
                          function_expression.get_variant());
    }
    FunctionSkeleton transform_impl(const FunctionSkeletonImpl& function_skeleton)
    {
        return this->m_pddl_repositories.get_or_create_function_skeleton(function_skeleton.get_name(), this->transform(function_skeleton.get_parameters()));
    }
    Function transform_impl(const FunctionImpl& function)
    {
        return this->m_pddl_repositories.get_or_create_function(this->transform(*function.get_function_skeleton()), this->transform(function.get_terms()));
    }
    GroundFunction transform_impl(const GroundFunctionImpl& function)
    {
        return this->m_pddl_repositories.get_or_create_ground_function(this->transform(*function.get_function_skeleton()),
                                                                       this->transform(function.get_objects()));
    }
    Action transform_impl(const ActionImpl& action)
    {
        return this->m_pddl_repositories.get_or_create_action(action.get_name(),
                                                              action.get_original_arity(),
                                                              this->transform(action.get_parameters()),
                                                              this->transform(action.get_conditions<Static>()),
                                                              this->transform(action.get_conditions<Fluent>()),
                                                              this->transform(action.get_conditions<Derived>()),
                                                              this->transform(action.get_strips_effect()),
                                                              this->transform(action.get_conditional_effects()));
    }
    Axiom transform_impl(const AxiomImpl& axiom)
    {
        return this->m_pddl_repositories.get_or_create_axiom(this->transform(axiom.get_parameters()),
                                                             this->transform(*axiom.get_literal()),
                                                             this->transform(axiom.get_conditions<Static>()),
                                                             this->transform(axiom.get_conditions<Fluent>()),
                                                             this->transform(axiom.get_conditions<Derived>()));
    }
    Domain transform_impl(const DomainImpl& domain)
    {
        return this->m_pddl_repositories.get_or_create_domain(domain.get_filepath(),
                                                              domain.get_name(),
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
        return this->m_pddl_repositories.get_or_create_optimization_metric(metric.get_optimization_metric(),
                                                                           this->transform(*metric.get_function_expression()));
    }

    Problem transform_impl(const ProblemImpl& problem)
    {
        return this->m_pddl_repositories.get_or_create_problem(
            problem.get_filepath(),
            this->transform(*problem.get_domain()),
            problem.get_name(),
            this->transform(*problem.get_requirements()),
            this->transform(problem.get_objects()),
            this->transform(problem.get_derived_predicates()),
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

    Problem run_impl(const ProblemImpl& problem)
    {
        this->prepare(problem);
        return this->transform(problem);
    }
};
}

#endif
