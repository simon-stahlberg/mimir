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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_BASE_CACHED_RECURSE_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_BASE_CACHED_RECURSE_HPP_

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
class BaseCachedRecurseTransformer : public ITransformer<BaseCachedRecurseTransformer<Derived_>>
{
private:
    BaseCachedRecurseTransformer() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    PDDLRepositories& m_pddl_repositories;

    template<typename T>
    using PDDLElementTranslationCache = boost::hana::pair<boost::hana::type<T>, std::unordered_map<T, T, loki::Hash<T>, loki::EqualTo<T>>>;

    using PDDLElementTranslationCaches = boost::hana::map<PDDLElementTranslationCache<Requirements>,
                                                          PDDLElementTranslationCache<Object>,
                                                          PDDLElementTranslationCache<Variable>,
                                                          PDDLElementTranslationCache<Term>,
                                                          PDDLElementTranslationCache<Predicate<Static>>,
                                                          PDDLElementTranslationCache<Predicate<Fluent>>,
                                                          PDDLElementTranslationCache<Predicate<Derived>>,
                                                          PDDLElementTranslationCache<Atom<Static>>,
                                                          PDDLElementTranslationCache<Atom<Fluent>>,
                                                          PDDLElementTranslationCache<Atom<Derived>>,
                                                          PDDLElementTranslationCache<GroundAtom<Static>>,
                                                          PDDLElementTranslationCache<GroundAtom<Fluent>>,
                                                          PDDLElementTranslationCache<GroundAtom<Derived>>,
                                                          PDDLElementTranslationCache<Literal<Static>>,
                                                          PDDLElementTranslationCache<Literal<Fluent>>,
                                                          PDDLElementTranslationCache<Literal<Derived>>,
                                                          PDDLElementTranslationCache<GroundLiteral<Static>>,
                                                          PDDLElementTranslationCache<GroundLiteral<Fluent>>,
                                                          PDDLElementTranslationCache<GroundLiteral<Derived>>,
                                                          PDDLElementTranslationCache<GroundFunctionValue<Static>>,
                                                          PDDLElementTranslationCache<GroundFunctionValue<Fluent>>,
                                                          PDDLElementTranslationCache<GroundFunctionValue<Auxiliary>>,
                                                          PDDLElementTranslationCache<NumericEffect<Fluent>>,
                                                          PDDLElementTranslationCache<NumericEffect<Auxiliary>>,
                                                          PDDLElementTranslationCache<ConjunctiveEffect>,
                                                          PDDLElementTranslationCache<ConditionalEffect>,
                                                          PDDLElementTranslationCache<FunctionExpressionNumber>,
                                                          PDDLElementTranslationCache<FunctionExpressionBinaryOperator>,
                                                          PDDLElementTranslationCache<FunctionExpressionMultiOperator>,
                                                          PDDLElementTranslationCache<FunctionExpressionMinus>,
                                                          PDDLElementTranslationCache<FunctionExpressionFunction<Static>>,
                                                          PDDLElementTranslationCache<FunctionExpressionFunction<Fluent>>,
                                                          PDDLElementTranslationCache<FunctionExpressionFunction<Auxiliary>>,
                                                          PDDLElementTranslationCache<FunctionExpression>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionNumber>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionBinaryOperator>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionMultiOperator>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionMinus>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionFunction<Static>>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionFunction<Fluent>>,
                                                          PDDLElementTranslationCache<GroundFunctionExpressionFunction<Auxiliary>>,
                                                          PDDLElementTranslationCache<GroundFunctionExpression>,
                                                          PDDLElementTranslationCache<FunctionSkeleton<Static>>,
                                                          PDDLElementTranslationCache<FunctionSkeleton<Fluent>>,
                                                          PDDLElementTranslationCache<FunctionSkeleton<Auxiliary>>,
                                                          PDDLElementTranslationCache<Function<Static>>,
                                                          PDDLElementTranslationCache<Function<Fluent>>,
                                                          PDDLElementTranslationCache<Function<Auxiliary>>,
                                                          PDDLElementTranslationCache<GroundFunction<Static>>,
                                                          PDDLElementTranslationCache<GroundFunction<Fluent>>,
                                                          PDDLElementTranslationCache<GroundFunction<Auxiliary>>,
                                                          PDDLElementTranslationCache<NumericConstraint>,
                                                          PDDLElementTranslationCache<GroundNumericConstraint>,
                                                          PDDLElementTranslationCache<ConjunctiveCondition>,
                                                          PDDLElementTranslationCache<Action>,
                                                          PDDLElementTranslationCache<Axiom>,
                                                          PDDLElementTranslationCache<Domain>,
                                                          PDDLElementTranslationCache<OptimizationMetric>,
                                                          PDDLElementTranslationCache<Problem>>;

    PDDLElementTranslationCaches m_translated_elements;

    explicit BaseCachedRecurseTransformer(PDDLRepositories& pddl_repositories) : m_pddl_repositories(pddl_repositories), m_translated_elements() {}

protected:
    /* Implement ITranslator interface */
    friend class ITransformer<BaseCachedRecurseTransformer<Derived_>>;

    template<std::ranges::forward_range Range>
    void prepare_base(const Range& input)
    {
        self().prepare_impl(input);
    }
    template<std::ranges::forward_range Range>
    void prepare_impl(const Range& input)
    {
        std::ranges::for_each(input, [this](auto&& arg) { this->prepare(arg); });
    }

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    template<typename T>
    void prepare_base(const std::optional<T>& element)
    {
        if (element.has_value())
        {
            self().prepare_impl(element.value());
        }
    }
    template<typename T>
    void prepare_base(const T& element)
    {
        self().prepare_impl(element);
    }

    void prepare_impl(Requirements requirements) {}
    void prepare_impl(Object object) {}
    void prepare_impl(Variable variable) {}
    void prepare_impl(Term term)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, term->get_variant());
    }
    template<PredicateTag P>
    void prepare_impl(Predicate<P> predicate)
    {
        this->prepare(predicate->get_parameters());
    }
    template<PredicateTag P>
    void prepare_impl(Atom<P> atom)
    {
        this->prepare(atom->get_predicate());
        this->prepare(atom->get_terms());
    }
    template<PredicateTag P>
    void prepare_impl(GroundAtom<P> atom)
    {
        this->prepare(atom->get_predicate());
        this->prepare(atom->get_objects());
    }
    template<PredicateTag P>
    void prepare_impl(Literal<P> literal)
    {
        this->prepare(literal->get_atom());
    }
    template<PredicateTag P>
    void prepare_impl(GroundLiteral<P> literal)
    {
        this->prepare(literal->get_atom());
    }
    template<FunctionTag F>
    void prepare_impl(GroundFunctionValue<F> function_value)
    {
        this->prepare(function_value->get_function());
    }
    template<DynamicFunctionTag F>
    void prepare_impl(NumericEffect<F> effect)
    {
        this->prepare(effect->get_function());
        this->prepare(effect->get_function_expression());
    }
    void prepare_impl(ConjunctiveEffect effect)
    {
        this->prepare(effect->get_parameters());
        this->prepare(effect->get_literals());
        this->prepare(effect->get_fluent_numeric_effects());
        this->prepare(effect->get_auxiliary_numeric_effect());
    }
    void prepare_impl(ConditionalEffect effect)
    {
        this->prepare(effect->get_conjunctive_condition());
        this->prepare(effect->get_conjunctive_effect());
    }
    void prepare_impl(NumericConstraint condition)
    {
        this->prepare(condition->get_left_function_expression());
        this->prepare(condition->get_right_function_expression());
    }
    void prepare_impl(GroundNumericConstraint condition)
    {
        this->prepare(condition->get_left_function_expression());
        this->prepare(condition->get_right_function_expression());
    }
    void prepare_impl(ConjunctiveCondition condition)
    {
        this->prepare(condition->get_parameters());
        this->prepare(condition->get_literals<Static>());
        this->prepare(condition->get_literals<Fluent>());
        this->prepare(condition->get_literals<Derived>());
        this->prepare(condition->get_numeric_constraints());
    }
    void prepare_impl(FunctionExpressionNumber function_expression) {}
    void prepare_impl(FunctionExpressionBinaryOperator function_expression)
    {
        this->prepare(function_expression->get_left_function_expression());
        this->prepare(function_expression->get_right_function_expression());
    }
    void prepare_impl(FunctionExpressionMultiOperator function_expression) { this->prepare(function_expression->get_function_expressions()); }
    void prepare_impl(FunctionExpressionMinus function_expression) { this->prepare(function_expression->get_function_expression()); }
    template<FunctionTag F>
    void prepare_impl(FunctionExpressionFunction<F> function_expression)
    {
        this->prepare(function_expression->get_function());
    }
    void prepare_impl(FunctionExpression function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression->get_variant());
    }
    void prepare_impl(GroundFunctionExpressionNumber function_expression) {}
    void prepare_impl(GroundFunctionExpressionBinaryOperator function_expression)
    {
        this->prepare(function_expression->get_left_function_expression());
        this->prepare(function_expression->get_right_function_expression());
    }
    void prepare_impl(GroundFunctionExpressionMultiOperator function_expression) { this->prepare(function_expression->get_function_expressions()); }
    void prepare_impl(GroundFunctionExpressionMinus function_expression) { this->prepare(function_expression->get_function_expression()); }
    template<FunctionTag F>
    void prepare_impl(GroundFunctionExpressionFunction<F> function_expression)
    {
        this->prepare(function_expression->get_function());
    }
    void prepare_impl(GroundFunctionExpression function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression->get_variant());
    }
    template<FunctionTag F>
    void prepare_impl(FunctionSkeleton<F> function_skeleton)
    {
        this->prepare(function_skeleton->get_parameters());
    }
    template<FunctionTag F>
    void prepare_impl(Function<F> function)
    {
        this->prepare(function->get_function_skeleton());
        this->prepare(function->get_terms());
    }
    template<FunctionTag F>
    void prepare_impl(GroundFunction<F> function)
    {
        this->prepare(function->get_function_skeleton());
        this->prepare(function->get_objects());
    }
    void prepare_impl(Action action)
    {
        this->prepare(action->get_conjunctive_condition());
        this->prepare(action->get_conjunctive_effect());
        this->prepare(action->get_conditional_effects());
    }
    void prepare_impl(Axiom axiom)
    {
        this->prepare(axiom->get_conjunctive_condition());
        this->prepare(axiom->get_literal());
    }
    void prepare_impl(Domain domain)
    {
        this->prepare(domain->get_requirements());
        this->prepare(domain->get_constants());
        this->prepare(domain->get_predicates<Static>());
        this->prepare(domain->get_predicates<Fluent>());
        this->prepare(domain->get_predicates<Derived>());
        this->prepare(domain->get_functions<Static>());
        this->prepare(domain->get_functions<Fluent>());
        this->prepare(domain->get_auxiliary_function());
        this->prepare(domain->get_actions());
        this->prepare(domain->get_axioms());
    }
    void prepare_impl(OptimizationMetric metric) { this->prepare(metric->get_function_expression()); }
    void prepare_impl(Problem problem)
    {
        this->prepare(problem->get_domain());
        this->prepare(problem->get_requirements());
        this->prepare(problem->get_objects());
        this->prepare(problem->get_derived_predicates());
        this->prepare(problem->get_static_initial_literals());
        this->prepare(problem->get_fluent_initial_literals());
        this->prepare(problem->get_function_values<Static>());
        this->prepare(problem->get_function_values<Fluent>());
        this->prepare(problem->get_auxiliary_function_value());
        this->prepare(problem->get_goal_condition<Static>());
        this->prepare(problem->get_goal_condition<Fluent>());
        this->prepare(problem->get_goal_condition<Derived>());
        this->prepare(problem->get_numeric_goal_condition());
        this->prepare(problem->get_optimization_metric());
        this->prepare(problem->get_axioms());
    }

    /// @brief Retrieve or create cache entry of translation to avoid recomputations.
    template<typename Impl, typename TransformFunc>
    auto cached_transform_impl(Impl impl, std::unordered_map<Impl, Impl, loki::Hash<Impl>, loki::EqualTo<Impl>>& cache, const TransformFunc& transformFunc)
    {
        // Access from cache
        auto it = cache.find(impl);
        if (it != cache.end())
        {
            return it->second;
        }

        // Translate
        auto transformed = transformFunc(impl);

        // Insert into cache
        cache.emplace(impl, transformed);

        return transformed;
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

        std::ranges::transform(input, std::back_inserter(output), [this](auto&& arg) { return this->transform(arg); });

        return output;
    }
    template<typename T>
    auto transform_base(const std::optional<T>& element)
    {
        return element.has_value() ? this->transform(element.value()) : std::optional<T> { std::nullopt };
    }
    template<typename T>
    auto transform_base(const T& element)
    {
        return cached_transform_impl(element,
                                     boost::hana::at_key(m_translated_elements, boost::hana::type<T> {}),
                                     [this](auto&& arg) { return self().transform_impl(arg); });
    }

    Requirements transform_impl(Requirements requirements) { return this->m_pddl_repositories.get_or_create_requirements(requirements->get_requirements()); }
    Object transform_impl(Object object) { return this->m_pddl_repositories.get_or_create_object(object->get_name()); }
    Variable transform_impl(Variable variable)
    {
        return this->m_pddl_repositories.get_or_create_variable(variable->get_name(), variable->get_parameter_index());
    }
    Term transform_impl(Term term)
    {
        return std::visit(
            [this](auto&& arg) -> Term
            {
                using ArgType = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<ArgType, Variable>)
                {
                    return this->m_pddl_repositories.get_or_create_term(this->transform(arg));
                }
                else if constexpr (std::is_same_v<ArgType, Object>)
                {
                    return this->m_pddl_repositories.get_or_create_term(this->transform(arg));
                }
                else
                {
                    static_assert(dependent_false<ArgType>::value, "Missing implementation for ArgType.");
                }
            },
            term->get_variant());
    }
    template<PredicateTag P>
    Predicate<P> transform_impl(Predicate<P> predicate)
    {
        return this->m_pddl_repositories.template get_or_create_predicate<P>(predicate->get_name(), this->transform(predicate->get_parameters()));
    }
    template<PredicateTag P>
    Atom<P> transform_impl(Atom<P> atom)
    {
        return this->m_pddl_repositories.get_or_create_atom(this->transform(atom->get_predicate()), this->transform(atom->get_terms()));
    }
    template<PredicateTag P>
    GroundAtom<P> transform_impl(GroundAtom<P> atom)
    {
        return this->m_pddl_repositories.get_or_create_ground_atom(this->transform(atom->get_predicate()), this->transform(atom->get_objects()));
    }
    template<PredicateTag P>
    Literal<P> transform_impl(Literal<P> literal)
    {
        return this->m_pddl_repositories.get_or_create_literal(literal->is_negated(), this->transform(literal->get_atom()));
    }
    template<PredicateTag P>
    GroundLiteral<P> transform_impl(GroundLiteral<P> literal)
    {
        return this->m_pddl_repositories.get_or_create_ground_literal(literal->is_negated(), this->transform(literal->get_atom()));
    }
    template<FunctionTag F>
    GroundFunctionValue<F> transform_impl(GroundFunctionValue<F> function_value)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_value(this->transform(function_value->get_function()), function_value->get_number());
    }
    template<DynamicFunctionTag F>
    NumericEffect<F> transform_impl(NumericEffect<F> effect)
    {
        return this->m_pddl_repositories.get_or_create_numeric_effect(effect->get_assign_operator(),
                                                                      this->transform(effect->get_function()),
                                                                      this->transform(effect->get_function_expression()));
    }
    ConjunctiveEffect transform_impl(ConjunctiveEffect effect)
    {
        return this->m_pddl_repositories.get_or_create_conjunctive_effect(this->transform(effect->get_parameters()),
                                                                          this->transform(effect->get_literals()),
                                                                          this->transform(effect->get_fluent_numeric_effects()),
                                                                          this->transform(effect->get_auxiliary_numeric_effect()));
    }
    ConditionalEffect transform_impl(ConditionalEffect effect)
    {
        return this->m_pddl_repositories.get_or_create_conditional_effect(this->transform(effect->get_conjunctive_condition()),
                                                                          this->transform(effect->get_conjunctive_effect()));
    }
    NumericConstraint transform_impl(NumericConstraint condition)
    {
        return this->m_pddl_repositories.get_or_create_numeric_constraint(condition->get_binary_comparator(),
                                                                          this->transform(condition->get_left_function_expression()),
                                                                          this->transform(condition->get_right_function_expression()));
    }
    GroundNumericConstraint transform_impl(GroundNumericConstraint condition)
    {
        return this->m_pddl_repositories.get_or_create_ground_numeric_constraint(condition->get_binary_comparator(),
                                                                                 this->transform(condition->get_left_function_expression()),
                                                                                 this->transform(condition->get_right_function_expression()));
    }
    ConjunctiveCondition transform_impl(ConjunctiveCondition condition)
    {
        return this->m_pddl_repositories.get_or_create_conjunctive_condition(this->transform(condition->get_parameters()),
                                                                             this->transform(condition->get_literals<Static>()),
                                                                             this->transform(condition->get_literals<Fluent>()),
                                                                             this->transform(condition->get_literals<Derived>()),
                                                                             this->transform(condition->get_numeric_constraints()));
    }
    FunctionExpressionNumber transform_impl(FunctionExpressionNumber function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_number(function_expression->get_number());
    }
    FunctionExpressionBinaryOperator transform_impl(FunctionExpressionBinaryOperator function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_binary_operator(
            function_expression->get_binary_operator(),
            this->transform(function_expression->get_left_function_expression()),
            this->transform(function_expression->get_right_function_expression()));
    }
    FunctionExpressionMultiOperator transform_impl(FunctionExpressionMultiOperator function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_multi_operator(function_expression->get_multi_operator(),
                                                                                          this->transform(function_expression->get_function_expressions()));
    }
    FunctionExpressionMinus transform_impl(FunctionExpressionMinus function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_minus(this->transform(function_expression->get_function_expression()));
    }
    template<FunctionTag F>
    FunctionExpressionFunction<F> transform_impl(FunctionExpressionFunction<F> function_expression)
    {
        return this->m_pddl_repositories.get_or_create_function_expression_function(this->transform(function_expression->get_function()));
    }
    FunctionExpression transform_impl(FunctionExpression function_expression)
    {
        return std::visit([this](auto&& arg) { return this->m_pddl_repositories.get_or_create_function_expression(this->transform(arg)); },
                          function_expression->get_variant());
    }
    GroundFunctionExpressionNumber transform_impl(GroundFunctionExpressionNumber function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_number(function_expression->get_number());
    }
    GroundFunctionExpressionBinaryOperator transform_impl(GroundFunctionExpressionBinaryOperator function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_binary_operator(
            function_expression->get_binary_operator(),
            this->transform(function_expression->get_left_function_expression()),
            this->transform(function_expression->get_right_function_expression()));
    }
    GroundFunctionExpressionMultiOperator transform_impl(GroundFunctionExpressionMultiOperator function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_multi_operator(
            function_expression->get_multi_operator(),
            this->transform(function_expression->get_function_expressions()));
    }
    GroundFunctionExpressionMinus transform_impl(GroundFunctionExpressionMinus function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_minus(this->transform(function_expression->get_function_expression()));
    }
    template<FunctionTag F>
    GroundFunctionExpressionFunction<F> transform_impl(GroundFunctionExpressionFunction<F> function_expression)
    {
        return this->m_pddl_repositories.get_or_create_ground_function_expression_function(this->transform(function_expression->get_function()));
    }
    GroundFunctionExpression transform_impl(GroundFunctionExpression function_expression)
    {
        return std::visit([this](auto&& arg) { return this->m_pddl_repositories.get_or_create_ground_function_expression(this->transform(arg)); },
                          function_expression->get_variant());
    }
    template<FunctionTag F>
    FunctionSkeleton<F> transform_impl(FunctionSkeleton<F> function_skeleton)
    {
        return this->m_pddl_repositories.template get_or_create_function_skeleton<F>(function_skeleton->get_name(),
                                                                                     this->transform(function_skeleton->get_parameters()));
    }
    template<FunctionTag F>
    Function<F> transform_impl(Function<F> function)
    {
        return this->m_pddl_repositories.get_or_create_function(this->transform(function->get_function_skeleton()),
                                                                this->transform(function->get_terms()),
                                                                function->get_parent_terms_to_terms_mapping());
    }
    template<FunctionTag F>
    GroundFunction<F> transform_impl(GroundFunction<F> function)
    {
        return this->m_pddl_repositories.get_or_create_ground_function(this->transform(function->get_function_skeleton()),
                                                                       this->transform(function->get_objects()));
    }
    Action transform_impl(Action action)
    {
        return this->m_pddl_repositories.get_or_create_action(action->get_name(),
                                                              action->get_original_arity(),
                                                              this->transform(action->get_conjunctive_condition()),
                                                              this->transform(action->get_conjunctive_effect()),
                                                              this->transform(action->get_conditional_effects()));
    }
    Axiom transform_impl(Axiom axiom)
    {
        return this->m_pddl_repositories.get_or_create_axiom(this->transform(axiom->get_conjunctive_condition()), this->transform(axiom->get_literal()));
    }
    Domain transform_impl(Domain domain)
    {
        return this->m_pddl_repositories.get_or_create_domain(domain->get_filepath(),
                                                              domain->get_name(),
                                                              this->transform(domain->get_requirements()),
                                                              this->transform(domain->get_constants()),
                                                              this->transform(domain->get_predicates<Static>()),
                                                              this->transform(domain->get_predicates<Fluent>()),
                                                              this->transform(domain->get_predicates<Derived>()),
                                                              this->transform(domain->get_functions<Static>()),
                                                              this->transform(domain->get_functions<Fluent>()),
                                                              this->transform(domain->get_auxiliary_function()),
                                                              this->transform(domain->get_actions()),
                                                              this->transform(domain->get_axioms()));
    }
    OptimizationMetric transform_impl(OptimizationMetric metric)
    {
        return this->m_pddl_repositories.get_or_create_optimization_metric(metric->get_optimization_metric(),
                                                                           this->transform(metric->get_function_expression()));
    }

    Problem transform_impl(Problem problem)
    {
        return this->m_pddl_repositories.get_or_create_problem(problem->get_filepath(),
                                                               this->transform(problem->get_domain()),
                                                               problem->get_name(),
                                                               this->transform(problem->get_requirements()),
                                                               this->transform(problem->get_objects()),
                                                               this->transform(problem->get_derived_predicates()),
                                                               this->transform(problem->get_static_initial_literals()),
                                                               this->transform(problem->get_fluent_initial_literals()),
                                                               this->transform(problem->get_function_values<Static>()),
                                                               this->transform(problem->get_function_values<Fluent>()),
                                                               this->transform(problem->get_auxiliary_function_value()),
                                                               this->transform(problem->get_goal_condition<Static>()),
                                                               this->transform(problem->get_goal_condition<Fluent>()),
                                                               this->transform(problem->get_goal_condition<Derived>()),
                                                               this->transform(problem->get_numeric_goal_condition()),
                                                               this->transform(problem->get_optimization_metric()),
                                                               this->transform(problem->get_axioms()));
    }

    /// @brief Recursively apply preparation followed by transformation.
    ///        Default behavior runs prepare and transform and returns its results.
    Problem run_base(Problem problem) { return self().run_impl(problem); }

    Problem run_impl(Problem problem)
    {
        this->prepare(problem);
        return this->transform(problem);
    }
};
}

#endif
