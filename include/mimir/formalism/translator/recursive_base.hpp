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

#ifndef MIMIR_FORMALISM_TRANSLATOR_RECURSIVE_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_RECURSIVE_BASE_HPP_

#include "mimir/formalism/concepts.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/domain_builder.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/problem_builder.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/translator/interface.hpp"

#include <loki/loki.hpp>
#include <unordered_map>

namespace mimir::formalism
{

/**
 * Base implementation recursively calls prepare, followed by recursively calls transform and caches the results.
 */
template<typename Derived_>
class RecursiveBaseTranslator : public ITranslator<RecursiveBaseTranslator<Derived_>>
{
private:
    RecursiveBaseTranslator() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    /* Implement ITranslator interface */
    friend class ITranslator<RecursiveBaseTranslator<Derived_>>;

    template<std::ranges::forward_range Range>
    void prepare_level_1(const Range& range)
    {
        self().prepare_level_2(range);
    }
    template<std::ranges::forward_range Range>
    void prepare_level_2(const Range& range)
    {
        std::ranges::for_each(range, [this](auto&& arg) { this->prepare_level_0(arg); });
    }

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    template<typename T>
    void prepare_level_1(const std::optional<T>& element)
    {
        if (element.has_value())
        {
            self().prepare_level_2(element.value());
        }
    }
    template<IsFormalismEntity T>
    void prepare_level_1(const T& element)
    {
        self().prepare_level_2(element);
    }

    void prepare_level_2(Requirements requirements) {}
    void prepare_level_2(Object object) {}
    void prepare_level_2(Variable variable) {}
    void prepare_level_2(Term term)
    {
        std::visit([this](auto&& arg) { return this->prepare_level_0(arg); }, term->get_variant());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    void prepare_level_2(Predicate<P> predicate)
    {
        this->prepare_level_0(predicate->get_parameters());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    void prepare_level_2(Atom<P> atom)
    {
        this->prepare_level_0(atom->get_predicate());
        this->prepare_level_0(atom->get_terms());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    void prepare_level_2(GroundAtom<P> atom)
    {
        this->prepare_level_0(atom->get_predicate());
        this->prepare_level_0(atom->get_objects());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    void prepare_level_2(Literal<P> literal)
    {
        this->prepare_level_0(literal->get_atom());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    void prepare_level_2(GroundLiteral<P> literal)
    {
        this->prepare_level_0(literal->get_atom());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(GroundFunctionValue<F> function_value)
    {
        this->prepare_level_0(function_value->get_function());
    }
    template<IsFluentOrAuxiliaryTag F>
    void prepare_level_2(NumericEffect<F> effect)
    {
        this->prepare_level_0(effect->get_function());
        this->prepare_level_0(effect->get_function_expression());
    }
    void prepare_level_2(ConjunctiveEffect effect)
    {
        this->prepare_level_0(effect->get_parameters());
        this->prepare_level_0(effect->get_literals());
        this->prepare_level_0(effect->get_fluent_numeric_effects());
        this->prepare_level_0(effect->get_auxiliary_numeric_effect());
    }
    void prepare_level_2(ConditionalEffect effect)
    {
        this->prepare_level_0(effect->get_conjunctive_condition());
        this->prepare_level_0(effect->get_conjunctive_effect());
    }
    void prepare_level_2(NumericConstraint condition)
    {
        this->prepare_level_0(condition->get_left_function_expression());
        this->prepare_level_0(condition->get_right_function_expression());
        this->prepare_level_0(condition->get_terms());
    }
    void prepare_level_2(GroundNumericConstraint condition)
    {
        this->prepare_level_0(condition->get_left_function_expression());
        this->prepare_level_0(condition->get_right_function_expression());
    }
    void prepare_level_2(ConjunctiveCondition condition)
    {
        this->prepare_level_0(condition->get_parameters());
        this->prepare_level_0(condition->get_literals<StaticTag>());
        this->prepare_level_0(condition->get_literals<FluentTag>());
        this->prepare_level_0(condition->get_literals<DerivedTag>());
        this->prepare_level_0(condition->get_numeric_constraints());
    }
    void prepare_level_2(FunctionExpressionNumber function_expression) {}
    void prepare_level_2(FunctionExpressionBinaryOperator function_expression)
    {
        this->prepare_level_0(function_expression->get_left_function_expression());
        this->prepare_level_0(function_expression->get_right_function_expression());
    }
    void prepare_level_2(FunctionExpressionMultiOperator function_expression) { this->prepare_level_0(function_expression->get_function_expressions()); }
    void prepare_level_2(FunctionExpressionMinus function_expression) { this->prepare_level_0(function_expression->get_function_expression()); }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(FunctionExpressionFunction<F> function_expression)
    {
        this->prepare_level_0(function_expression->get_function());
    }
    void prepare_level_2(FunctionExpression function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare_level_0(arg); }, function_expression->get_variant());
    }
    void prepare_level_2(GroundFunctionExpressionNumber function_expression) {}
    void prepare_level_2(GroundFunctionExpressionBinaryOperator function_expression)
    {
        this->prepare_level_0(function_expression->get_left_function_expression());
        this->prepare_level_0(function_expression->get_right_function_expression());
    }
    void prepare_level_2(GroundFunctionExpressionMultiOperator function_expression) { this->prepare_level_0(function_expression->get_function_expressions()); }
    void prepare_level_2(GroundFunctionExpressionMinus function_expression) { this->prepare_level_0(function_expression->get_function_expression()); }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(GroundFunctionExpressionFunction<F> function_expression)
    {
        this->prepare_level_0(function_expression->get_function());
    }
    void prepare_level_2(GroundFunctionExpression function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare_level_0(arg); }, function_expression->get_variant());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(FunctionSkeleton<F> function_skeleton)
    {
        this->prepare_level_0(function_skeleton->get_parameters());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(Function<F> function)
    {
        this->prepare_level_0(function->get_function_skeleton());
        this->prepare_level_0(function->get_terms());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    void prepare_level_2(GroundFunction<F> function)
    {
        this->prepare_level_0(function->get_function_skeleton());
        this->prepare_level_0(function->get_objects());
    }
    void prepare_level_2(Action action)
    {
        this->prepare_level_0(action->get_conjunctive_condition());
        this->prepare_level_0(action->get_conjunctive_effect());
        this->prepare_level_0(action->get_conditional_effects());
    }
    void prepare_level_2(Axiom axiom)
    {
        this->prepare_level_0(axiom->get_conjunctive_condition());
        this->prepare_level_0(axiom->get_literal());
    }
    void prepare_level_2(Domain domain)
    {
        this->prepare_level_0(domain->get_requirements());
        this->prepare_level_0(domain->get_constants());
        this->prepare_level_0(domain->get_predicates<StaticTag>());
        this->prepare_level_0(domain->get_predicates<FluentTag>());
        this->prepare_level_0(domain->get_predicates<DerivedTag>());
        this->prepare_level_0(domain->get_function_skeletons<StaticTag>());
        this->prepare_level_0(domain->get_function_skeletons<FluentTag>());
        this->prepare_level_0(domain->get_auxiliary_function_skeleton());
        this->prepare_level_0(domain->get_actions());
        this->prepare_level_0(domain->get_axioms());
    }
    void prepare_level_2(OptimizationMetric metric) { this->prepare(metric->get_function_expression()); }
    void prepare_level_2(Problem problem)
    {
        this->prepare_level_0(problem->get_domain());
        this->prepare_level_0(problem->get_requirements());
        this->prepare_level_0(problem->get_objects());
        this->prepare_level_0(problem->get_derived_predicates());
        this->prepare_level_0(problem->get_initial_literals<StaticTag>());
        this->prepare_level_0(problem->get_initial_literals<FluentTag>());
        this->prepare_level_0(problem->get_initial_function_values<StaticTag>());
        this->prepare_level_0(problem->get_initial_function_values<FluentTag>());
        this->prepare_level_0(problem->get_auxiliary_function_value());
        this->prepare_level_0(problem->get_goal_condition<StaticTag>());
        this->prepare_level_0(problem->get_goal_condition<FluentTag>());
        this->prepare_level_0(problem->get_goal_condition<DerivedTag>());
        this->prepare_level_0(problem->get_numeric_goal_condition());
        this->prepare_level_0(problem->get_optimization_metric());
        this->prepare_level_0(problem->get_axioms());
    }

    /// @brief Translate a container of elements into a container of elements.
    template<IsBackInsertibleRange Range>
    auto translate_level_1(const Range& input, Repositories& repositories)
    {
        return self().translate_level_2(input, repositories);
    }
    /// @brief Translate a container of elements into a container of elements.
    template<IsBackInsertibleRange Range>
    auto translate_level_2(const Range& input, Repositories& repositories)
    {
        std::remove_cvref_t<Range> output;

        if constexpr (requires { output.reserve(std::ranges::size(input)); })
        {
            output.reserve(std::ranges::size(input));
        }

        std::ranges::transform(input, std::back_inserter(output), [&](auto&& arg) { return this->translate_level_0(arg, repositories); });

        return output;
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_repositories.
    template<typename T>
    auto translate_level_1(const std::optional<T>& element, Repositories& repositories)
    {
        return element.has_value() ? this->translate_level_0(element.value(), repositories) : std::optional<T> { std::nullopt };
    }
    template<IsHanaMap T>
    auto translate_level_1(const T& map, Repositories& repositories)
    {
        return boost::hana::unpack(
            map,
            [&](auto... pairs) {
                return boost::hana::make_map(
                    boost::hana::make_pair(boost::hana::first(pairs), self().translate_level_0(boost::hana::second(pairs), repositories))...);
            });
    }
    template<IsFormalismEntity T>
    auto translate_level_1(const T& element, Repositories& repositories)
    {
        return self().translate_level_2(element, repositories);
    }

    Requirements translate_level_2(Requirements requirements, Repositories& repositories)
    {
        return repositories.get_or_create_requirements(requirements->get_requirements());
    }
    Object translate_level_2(Object object, Repositories& repositories) { return repositories.get_or_create_object(object->get_name()); }
    Variable translate_level_2(Variable variable, Repositories& repositories)
    {
        return repositories.get_or_create_variable(variable->get_name(), variable->get_parameter_index());
    }
    Term translate_level_2(Term term, Repositories& repositories)
    {
        return std::visit([&](auto&& arg) -> Term { return repositories.get_or_create_term(this->translate_level_0(arg, repositories)); }, term->get_variant());
    }
    template<IsStaticOrFluentOrDerivedTag P>
    Predicate<P> translate_level_2(Predicate<P> predicate, Repositories& repositories)
    {
        return repositories.template get_or_create_predicate<P>(predicate->get_name(), this->translate_level_0(predicate->get_parameters(), repositories));
    }
    template<IsStaticOrFluentOrDerivedTag P>
    Atom<P> translate_level_2(Atom<P> atom, Repositories& repositories)
    {
        return repositories.get_or_create_atom(this->translate_level_0(atom->get_predicate(), repositories),
                                               this->translate_level_0(atom->get_terms(), repositories));
    }
    template<IsStaticOrFluentOrDerivedTag P>
    GroundAtom<P> translate_level_2(GroundAtom<P> atom, Repositories& repositories)
    {
        return repositories.get_or_create_ground_atom(this->translate_level_0(atom->get_predicate(), repositories),
                                                      this->translate_level_0(atom->get_objects(), repositories));
    }
    template<IsStaticOrFluentOrDerivedTag P>
    Literal<P> translate_level_2(Literal<P> literal, Repositories& repositories)
    {
        return repositories.get_or_create_literal(literal->get_polarity(), this->translate_level_0(literal->get_atom(), repositories));
    }
    template<IsStaticOrFluentOrDerivedTag P>
    GroundLiteral<P> translate_level_2(GroundLiteral<P> literal, Repositories& repositories)
    {
        return repositories.get_or_create_ground_literal(literal->get_polarity(), this->translate_level_0(literal->get_atom(), repositories));
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunctionValue<F> translate_level_2(GroundFunctionValue<F> function_value, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_value(this->translate_level_0(function_value->get_function(), repositories),
                                                                function_value->get_number());
    }
    template<IsFluentOrAuxiliaryTag F>
    NumericEffect<F> translate_level_2(NumericEffect<F> effect, Repositories& repositories)
    {
        return repositories.get_or_create_numeric_effect(effect->get_assign_operator(),
                                                         this->translate_level_0(effect->get_function(), repositories),
                                                         this->translate_level_0(effect->get_function_expression(), repositories));
    }
    ConjunctiveEffect translate_level_2(ConjunctiveEffect effect, Repositories& repositories)
    {
        return repositories.get_or_create_conjunctive_effect(this->translate_level_0(effect->get_parameters(), repositories),
                                                             this->translate_level_0(effect->get_literals(), repositories),
                                                             this->translate_level_0(effect->get_fluent_numeric_effects(), repositories),
                                                             this->translate_level_0(effect->get_auxiliary_numeric_effect(), repositories));
    }
    ConditionalEffect translate_level_2(ConditionalEffect effect, Repositories& repositories)
    {
        return repositories.get_or_create_conditional_effect(this->translate_level_0(effect->get_conjunctive_condition(), repositories),
                                                             this->translate_level_0(effect->get_conjunctive_effect(), repositories));
    }
    NumericConstraint translate_level_2(NumericConstraint condition, Repositories& repositories)
    {
        return repositories.get_or_create_numeric_constraint(condition->get_binary_comparator(),
                                                             this->translate_level_0(condition->get_left_function_expression(), repositories),
                                                             this->translate_level_0(condition->get_right_function_expression(), repositories),
                                                             this->translate_level_0(condition->get_terms(), repositories));
    }
    GroundNumericConstraint translate_level_2(GroundNumericConstraint condition, Repositories& repositories)
    {
        return repositories.get_or_create_ground_numeric_constraint(condition->get_binary_comparator(),
                                                                    this->translate_level_0(condition->get_left_function_expression(), repositories),
                                                                    this->translate_level_0(condition->get_right_function_expression(), repositories));
    }
    ConjunctiveCondition translate_level_2(ConjunctiveCondition condition, Repositories& repositories)
    {
        return repositories.get_or_create_conjunctive_condition(this->translate_level_0(condition->get_parameters(), repositories),
                                                                this->translate_level_0(condition->get_hana_literals(), repositories),
                                                                this->translate_level_0(condition->get_numeric_constraints(), repositories));
    }
    FunctionExpressionNumber translate_level_2(FunctionExpressionNumber function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_function_expression_number(function_expression->get_number());
    }
    FunctionExpressionBinaryOperator translate_level_2(FunctionExpressionBinaryOperator function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_function_expression_binary_operator(
            function_expression->get_binary_operator(),
            this->translate_level_0(function_expression->get_left_function_expression(), repositories),
            this->translate_level_0(function_expression->get_right_function_expression(), repositories));
    }
    FunctionExpressionMultiOperator translate_level_2(FunctionExpressionMultiOperator function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_function_expression_multi_operator(
            function_expression->get_multi_operator(),
            this->translate_level_0(function_expression->get_function_expressions(), repositories));
    }
    FunctionExpressionMinus translate_level_2(FunctionExpressionMinus function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_function_expression_minus(this->translate_level_0(function_expression->get_function_expression(), repositories));
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    FunctionExpressionFunction<F> translate_level_2(FunctionExpressionFunction<F> function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_function_expression_function(this->translate_level_0(function_expression->get_function(), repositories));
    }
    FunctionExpression translate_level_2(FunctionExpression function_expression, Repositories& repositories)
    {
        return std::visit([&](auto&& arg) { return repositories.get_or_create_function_expression(this->translate_level_0(arg, repositories)); },
                          function_expression->get_variant());
    }
    GroundFunctionExpressionNumber translate_level_2(GroundFunctionExpressionNumber function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_expression_number(function_expression->get_number());
    }
    GroundFunctionExpressionBinaryOperator translate_level_2(GroundFunctionExpressionBinaryOperator function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_expression_binary_operator(
            function_expression->get_binary_operator(),
            this->translate_level_0(function_expression->get_left_function_expression(), repositories),
            this->translate_level_0(function_expression->get_right_function_expression(), repositories));
    }
    GroundFunctionExpressionMultiOperator translate_level_2(GroundFunctionExpressionMultiOperator function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_expression_multi_operator(
            function_expression->get_multi_operator(),
            this->translate_level_0(function_expression->get_function_expressions(), repositories));
    }
    GroundFunctionExpressionMinus translate_level_2(GroundFunctionExpressionMinus function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_expression_minus(
            this->translate_level_0(function_expression->get_function_expression(), repositories));
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunctionExpressionFunction<F> translate_level_2(GroundFunctionExpressionFunction<F> function_expression, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function_expression_function(this->translate_level_0(function_expression->get_function(), repositories));
    }
    GroundFunctionExpression translate_level_2(GroundFunctionExpression function_expression, Repositories& repositories)
    {
        return std::visit([&](auto&& arg) -> GroundFunctionExpression
                          { return repositories.get_or_create_ground_function_expression(this->translate_level_0(arg, repositories)); },
                          function_expression->get_variant());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    FunctionSkeleton<F> translate_level_2(FunctionSkeleton<F> function_skeleton, Repositories& repositories)
    {
        return repositories.template get_or_create_function_skeleton<F>(function_skeleton->get_name(),
                                                                        this->translate_level_0(function_skeleton->get_parameters(), repositories));
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    Function<F> translate_level_2(Function<F> function, Repositories& repositories)
    {
        return repositories.get_or_create_function(this->translate_level_0(function->get_function_skeleton(), repositories),
                                                   this->translate_level_0(function->get_terms(), repositories),
                                                   function->get_parent_terms_to_terms_mapping());
    }
    template<IsStaticOrFluentOrAuxiliaryTag F>
    GroundFunction<F> translate_level_2(GroundFunction<F> function, Repositories& repositories)
    {
        return repositories.get_or_create_ground_function(this->translate_level_0(function->get_function_skeleton(), repositories),
                                                          this->translate_level_0(function->get_objects(), repositories));
    }
    Action translate_level_2(Action action, Repositories& repositories)
    {
        return repositories.get_or_create_action(action->get_name(),
                                                 action->get_original_arity(),
                                                 this->translate_level_0(action->get_conjunctive_condition(), repositories),
                                                 this->translate_level_0(action->get_conjunctive_effect(), repositories),
                                                 this->translate_level_0(action->get_conditional_effects(), repositories));
    }
    Axiom translate_level_2(Axiom axiom, Repositories& repositories)
    {
        return repositories.get_or_create_axiom(this->translate_level_0(axiom->get_conjunctive_condition(), repositories),
                                                this->translate_level_0(axiom->get_literal(), repositories));
    }
    OptimizationMetric translate_level_2(OptimizationMetric metric, Repositories& repositories)
    {
        return repositories.get_or_create_optimization_metric(metric->get_optimization_metric(),
                                                              this->translate_level_0(metric->get_function_expression(), repositories));
    }

    auto translate_level_1(const Domain& domain, DomainBuilder& builder) { return self().translate_level_2(domain, builder); }

    auto translate_level_2(const Domain& domain, DomainBuilder& builder)
    {
        auto& repositories = builder.get_repositories();

        builder.get_name() = domain->get_name();
        builder.get_filepath() = domain->get_filepath();
        builder.get_requirements() = this->translate_level_0(domain->get_requirements(), repositories);
        builder.get_constants() = this->translate_level_0(domain->get_constants(), repositories);
        builder.get_predicates<StaticTag>() = this->translate_level_0(domain->get_predicates<StaticTag>(), repositories);
        builder.get_predicates<FluentTag>() = this->translate_level_0(domain->get_predicates<FluentTag>(), repositories);
        builder.get_predicates<DerivedTag>() = this->translate_level_0(domain->get_predicates<DerivedTag>(), repositories);
        builder.get_function_skeletons<StaticTag>() = this->translate_level_0(domain->get_function_skeletons<StaticTag>(), repositories);
        builder.get_function_skeletons<FluentTag>() = this->translate_level_0(domain->get_function_skeletons<FluentTag>(), repositories);
        builder.get_auxiliary_function_skeleton() = this->translate_level_0(domain->get_auxiliary_function_skeleton(), repositories);
        builder.get_actions() = this->translate_level_0(domain->get_actions(), repositories);
        builder.get_axioms() = this->translate_level_0(domain->get_axioms(), repositories);

        return builder.get_result();
    }

    auto translate_level_1(const Problem& problem, ProblemBuilder& builder) { return self().translate_level_2(problem, builder); }

    auto translate_level_2(const Problem& problem, ProblemBuilder& builder)
    {
        auto& repositories = builder.get_repositories();

        builder.get_filepath() = problem->get_filepath();
        builder.get_name() = problem->get_name();
        builder.get_requirements() = this->translate_level_0(problem->get_requirements(), repositories);
        builder.get_objects() = this->translate_level_0(problem->get_objects(), repositories);
        builder.get_derived_predicates() = this->translate_level_0(problem->get_derived_predicates(), repositories);
        builder.get_initial_literals<StaticTag>() = this->translate_level_0(problem->get_initial_literals<StaticTag>(), repositories);
        builder.get_initial_literals<FluentTag>() = this->translate_level_0(problem->get_initial_literals<FluentTag>(), repositories);
        builder.get_initial_function_values<StaticTag>() = this->translate_level_0(problem->get_initial_function_values<StaticTag>(), repositories);
        builder.get_initial_function_values<FluentTag>() = this->translate_level_0(problem->get_initial_function_values<FluentTag>(), repositories);
        builder.get_auxiliary_function_value() = this->translate_level_0(problem->get_auxiliary_function_value(), repositories);
        builder.get_goal_condition<StaticTag>() = this->translate_level_0(problem->get_goal_condition<StaticTag>(), repositories);
        builder.get_goal_condition<FluentTag>() = this->translate_level_0(problem->get_goal_condition<FluentTag>(), repositories);
        builder.get_goal_condition<DerivedTag>() = this->translate_level_0(problem->get_goal_condition<DerivedTag>(), repositories);
        builder.get_numeric_goal_condition() = this->translate_level_0(problem->get_numeric_goal_condition(), repositories);
        builder.get_optimization_metric() = this->translate_level_0(problem->get_optimization_metric(), repositories);
        builder.get_axioms() = this->translate_level_0(problem->get_axioms(), repositories);

        return builder.get_result(problem->get_index());
    }
};
}

#endif
