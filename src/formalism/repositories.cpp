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

#include "mimir/formalism/repositories.hpp"

#include "mimir/common/itertools.hpp"

namespace mimir
{

template<StaticOrFluentOrDerived P>
static GroundLiteralList<P> ground_nullary_literals(const LiteralList<P>& literals, PDDLRepositories& pddl_repositories)
{
    auto ground_literals = GroundLiteralList<P> {};
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_arity() != 0)
            continue;

        ground_literals.push_back(
            pddl_repositories.get_or_create_ground_literal(literal->is_negated(),
                                                           pddl_repositories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), {})));
    }

    return ground_literals;
}

static GroundNumericConstraintList ground_nullary_numeric_constraints(const NumericConstraintList& numeric_constraints, PDDLRepositories& pddl_repositories)
{
    auto ground_numeric_constraints = GroundNumericConstraintList {};
    for (const auto& numeric_constraint : numeric_constraints)
    {
        // TODO need to access problem
        // ground_numeric_constraints.push_back(pddl_repositories.ground(numeric_constraint, problem, ObjectList {}));
    }

    return ground_numeric_constraints;
}

PDDLRepositories::PDDLRepositories() : m_repositories() {}

PDDLRepositories::PDDLRepositories(PDDLRepositories&& other) = default;

PDDLRepositories& PDDLRepositories::operator=(PDDLRepositories&& other) = default;

Requirements PDDLRepositories::get_or_create_requirements(loki::RequirementEnumSet requirement_set)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RequirementsImpl> {}).get_or_create(std::move(requirement_set));
}

Variable PDDLRepositories::get_or_create_variable(std::string name, size_t parameter_index)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<VariableImpl> {}).get_or_create(std::move(name), std::move(parameter_index));
}

Term PDDLRepositories::get_or_create_term(Variable variable)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(variable));
}

Term PDDLRepositories::get_or_create_term(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(object));
}

Object PDDLRepositories::get_or_create_object(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).get_or_create(std::move(name));
}

template<StaticOrFluentOrDerived P>
Atom<P> PDDLRepositories::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AtomImpl<P>> {}).get_or_create(std::move(predicate), std::move(terms));
}

template Atom<Static> PDDLRepositories::get_or_create_atom(Predicate<Static> predicate, TermList terms);
template Atom<Fluent> PDDLRepositories::get_or_create_atom(Predicate<Fluent> predicate, TermList terms);
template Atom<Derived> PDDLRepositories::get_or_create_atom(Predicate<Derived> predicate, TermList terms);

template<StaticOrFluentOrDerived P>
GroundAtom<P> PDDLRepositories::get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).get_or_create(std::move(predicate), std::move(objects));
}

template GroundAtom<Static> PDDLRepositories::get_or_create_ground_atom(Predicate<Static> predicate, ObjectList ObjectList);
template GroundAtom<Fluent> PDDLRepositories::get_or_create_ground_atom(Predicate<Fluent> predicate, ObjectList ObjectList);
template GroundAtom<Derived> PDDLRepositories::get_or_create_ground_atom(Predicate<Derived> predicate, ObjectList ObjectList);

template<StaticOrFluentOrDerived P>
Literal<P> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<LiteralImpl<P>> {}).get_or_create(is_negated, std::move(atom));
}

template Literal<Static> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Static> atom);
template Literal<Fluent> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Fluent> atom);
template Literal<Derived> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Derived> atom);

template<StaticOrFluentOrDerived P>
GroundLiteral<P> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundLiteralImpl<P>> {}).get_or_create(is_negated, std::move(atom));
}

template GroundLiteral<Static> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Static> atom);
template GroundLiteral<Fluent> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Fluent> atom);
template GroundLiteral<Derived> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Derived> atom);

template<StaticOrFluentOrDerived P>
Predicate<P> PDDLRepositories::get_or_create_predicate(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PredicateImpl<P>> {}).get_or_create(name, std::move(parameters));
}

template Predicate<Static> PDDLRepositories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Fluent> PDDLRepositories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Derived> PDDLRepositories::get_or_create_predicate(std::string name, VariableList parameters);

FunctionExpressionNumber PDDLRepositories::get_or_create_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionNumberImpl> {}).get_or_create(number);
}

FunctionExpressionBinaryOperator PDDLRepositories::get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                     FunctionExpression left_function_expression,
                                                                                                     FunctionExpression right_function_expression)
{
    /* Canonize before uniqueness test. */
    if (binary_operator == loki::BinaryOperatorEnum::MUL || binary_operator == loki::BinaryOperatorEnum::PLUS)
    {
        if (left_function_expression->get_index() > right_function_expression->get_index())
            std::swap(left_function_expression, right_function_expression);
    }

    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionBinaryOperatorImpl> {})
        .get_or_create(binary_operator, std::move(left_function_expression), std::move(right_function_expression));
}

FunctionExpressionMultiOperator PDDLRepositories::get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                                   FunctionExpressionList function_expressions)
{
    /* Canonize before uniqueness test. */
    std::sort(function_expressions.begin(), function_expressions.end(), [](auto&& l, auto&& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionMultiOperatorImpl> {})
        .get_or_create(multi_operator, std::move(function_expressions));
}

FunctionExpressionMinus PDDLRepositories::get_or_create_function_expression_minus(FunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionMinusImpl> {}).get_or_create(std::move(function_expression));
}

template<StaticOrFluentTag F>
FunctionExpressionFunction<F> PDDLRepositories::get_or_create_function_expression_function(Function<F> function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionFunctionImpl<F>> {}).get_or_create(std::move(function));
}

template FunctionExpressionFunction<Static> PDDLRepositories::get_or_create_function_expression_function(Function<Static> function);
template FunctionExpressionFunction<Fluent> PDDLRepositories::get_or_create_function_expression_function(Function<Fluent> function);

FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionNumber fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionMultiOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionMinus fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

template<StaticOrFluentTag F>
FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionFunction<F> fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

template FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionFunction<Static> fexpr);
template FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionFunction<Fluent> fexpr);

GroundFunctionExpressionNumber PDDLRepositories::get_or_create_ground_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionNumberImpl> {}).get_or_create(number);
}

GroundFunctionExpressionBinaryOperator
PDDLRepositories::get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                           GroundFunctionExpression left_function_expression,
                                                                           GroundFunctionExpression right_function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionBinaryOperatorImpl> {})
        .get_or_create(binary_operator, std::move(left_function_expression), std::move(right_function_expression));
}

GroundFunctionExpressionMultiOperator
PDDLRepositories::get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                          GroundFunctionExpressionList function_expressions)
{
    /* Canonize before uniqueness test. */
    std::sort(function_expressions.begin(),
              function_expressions.end(),
              [](auto&& l, auto&& r)
              {
                  return std::visit([](auto&& arg) { return arg->get_index(); }, l->get_variant())
                         < std::visit([](const auto& arg) { return arg->get_index(); }, r->get_variant());
              });

    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionMultiOperatorImpl> {})
        .get_or_create(multi_operator, std::move(function_expressions));
}

GroundFunctionExpressionMinus PDDLRepositories::get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionMinusImpl> {}).get_or_create(std::move(function_expression));
}

template<FluentOrAuxiliary F>
GroundFunctionExpressionFunction<F> PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction<F> function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionFunctionImpl<F>> {}).get_or_create(std::move(function));
}

template GroundFunctionExpressionFunction<Fluent> PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction<Fluent> function);
template GroundFunctionExpressionFunction<Auxiliary> PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction<Auxiliary> function);

GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionNumber fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionBinaryOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionMultiOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionMinus fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

template<FluentOrAuxiliary F>
GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<F> fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}
template GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<Fluent> fexpr);
template GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<Auxiliary> fexpr);

template<StaticOrFluentOrAuxiliary F>
Function<F> PDDLRepositories::get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionImpl<F>> {})
        .get_or_create(std::move(function_skeleton), std::move(terms), std::move(m_parent_terms_to_terms_mapping));
}

template Function<Static>
PDDLRepositories::get_or_create_function(FunctionSkeleton<Static> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);
template Function<Fluent>
PDDLRepositories::get_or_create_function(FunctionSkeleton<Fluent> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);
template Function<Auxiliary>
PDDLRepositories::get_or_create_function(FunctionSkeleton<Auxiliary> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);

template<StaticOrFluentOrAuxiliary F>
GroundFunction<F> PDDLRepositories::get_or_create_ground_function(FunctionSkeleton<F> function_skeleton, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {}).get_or_create(std::move(function_skeleton), std::move(objects));
}

template GroundFunction<Static> PDDLRepositories::get_or_create_ground_function(FunctionSkeleton<Static> function_skeleton, ObjectList objects);
template GroundFunction<Fluent> PDDLRepositories::get_or_create_ground_function(FunctionSkeleton<Fluent> function_skeleton, ObjectList objects);
template GroundFunction<Auxiliary> PDDLRepositories::get_or_create_ground_function(FunctionSkeleton<Auxiliary> function_skeleton, ObjectList objects);

template<StaticOrFluentOrAuxiliary F>
FunctionSkeleton<F> PDDLRepositories::get_or_create_function_skeleton(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionSkeletonImpl<F>> {}).get_or_create(std::move(name), std::move(parameters));
}

template FunctionSkeleton<Static> PDDLRepositories::get_or_create_function_skeleton(std::string name, VariableList parameters);
template FunctionSkeleton<Fluent> PDDLRepositories::get_or_create_function_skeleton(std::string name, VariableList parameters);
template FunctionSkeleton<Auxiliary> PDDLRepositories::get_or_create_function_skeleton(std::string name, VariableList parameters);

template<FluentOrAuxiliary F>
NumericEffect<F>
PDDLRepositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericEffectImpl<F>> {})
        .get_or_create(std::move(assign_operator), std::move(function), std::move(function_expression));
}

template NumericEffect<Fluent>
PDDLRepositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<Fluent> function, FunctionExpression function_expression);
template NumericEffect<Auxiliary>
PDDLRepositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<Auxiliary> function, FunctionExpression function_expression);

ConjunctiveEffect PDDLRepositories::get_or_create_conjunctive_effect(VariableList parameters,
                                                                     LiteralList<Fluent> effects,
                                                                     NumericEffectList<Fluent> fluent_numeric_effects,
                                                                     std::optional<NumericEffect<Auxiliary>> auxiliary_numeric_effect)
{
    /* Canonize before uniqueness test. */
    std::sort(effects.begin(), effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_numeric_effects.begin(), fluent_numeric_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveEffectImpl> {})
        .get_or_create(std::move(parameters), std::move(effects), std::move(fluent_numeric_effects), std::move(auxiliary_numeric_effect));
}

ConditionalEffect PDDLRepositories::get_or_create_conditional_effect(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConditionalEffectImpl> {})
        .get_or_create(std::move(conjunctive_condition), std::move(conjunctive_effect));
}

NumericConstraint PDDLRepositories::get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                     FunctionExpression function_expression_left,
                                                                     FunctionExpression function_expression_right,
                                                                     TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericConstraintImpl> {})
        .get_or_create(std::move(binary_comparator), std::move(function_expression_left), std::move(function_expression_right), std::move(terms));
}

GroundNumericConstraint PDDLRepositories::get_or_create_ground_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                                  GroundFunctionExpression function_expression_left,
                                                                                  GroundFunctionExpression function_expression_right)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericConstraintImpl> {})
        .get_or_create(std::move(binary_comparator), std::move(function_expression_left), std::move(function_expression_right));
}

ConjunctiveCondition PDDLRepositories::get_or_create_conjunctive_condition(VariableList parameters,
                                                                           LiteralList<Static> static_conditions,
                                                                           LiteralList<Fluent> fluent_conditions,
                                                                           LiteralList<Derived> derived_conditions,
                                                                           NumericConstraintList numeric_constraints)
{
    /* Canonize before uniqueness test */
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(numeric_constraints.begin(), numeric_constraints.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    auto nullary_static_conditions = ground_nullary_literals(static_conditions, *this);
    auto nullary_fluent_conditions = ground_nullary_literals(fluent_conditions, *this);
    auto nullary_derived_conditions = ground_nullary_literals(derived_conditions, *this);

    std::sort(nullary_static_conditions.begin(), nullary_static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(nullary_fluent_conditions.begin(), nullary_fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(nullary_derived_conditions.begin(),
              nullary_derived_conditions.end(),
              [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    auto nullary_numeric_constraints = ground_nullary_numeric_constraints(numeric_constraints, *this);

    std::sort(nullary_numeric_constraints.begin(),
              nullary_numeric_constraints.end(),
              [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveConditionImpl> {})
        .get_or_create(std::move(parameters),
                       std::move(static_conditions),
                       std::move(fluent_conditions),
                       std::move(derived_conditions),
                       std::move(nullary_static_conditions),
                       std::move(nullary_fluent_conditions),
                       std::move(nullary_derived_conditions),
                       std::move(numeric_constraints),
                       std::move(nullary_numeric_constraints));
}

Action PDDLRepositories::get_or_create_action(std::string name,
                                              size_t original_arity,
                                              ConjunctiveCondition conjunctive_condition,
                                              ConjunctiveEffect conjunctive_effect,
                                              ConditionalEffectList conditional_effects)
{
    /* Canonize before uniqueness test */
    std::sort(conditional_effects.begin(), conditional_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {})
        .get_or_create(std::move(name), original_arity, std::move(conjunctive_condition), std::move(conjunctive_effect), std::move(conditional_effects));
}

Axiom PDDLRepositories::get_or_create_axiom(ConjunctiveCondition conjunctive_condition, Literal<Derived> literal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).get_or_create(std::move(conjunctive_condition), std::move(literal));
}

OptimizationMetric PDDLRepositories::get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<OptimizationMetricImpl> {}).get_or_create(std::move(metric), std::move(function_expression));
}

template<StaticOrFluentOrAuxiliary F>
GroundFunctionValue<F> PDDLRepositories::get_or_create_ground_function_value(GroundFunction<F> function, double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionValueImpl<F>> {}).get_or_create(std::move(function), std::move(number));
}

template GroundFunctionValue<Static> PDDLRepositories::get_or_create_ground_function_value(GroundFunction<Static> function, double number);
template GroundFunctionValue<Fluent> PDDLRepositories::get_or_create_ground_function_value(GroundFunction<Fluent> function, double number);
template GroundFunctionValue<Auxiliary> PDDLRepositories::get_or_create_ground_function_value(GroundFunction<Auxiliary> function, double number);

Domain PDDLRepositories::get_or_create_domain(std::optional<fs::path> filepath,
                                              std::string name,
                                              Requirements requirements,
                                              ObjectList constants,
                                              PredicateList<Static> static_predicates,
                                              PredicateList<Fluent> fluent_predicates,
                                              PredicateList<Derived> derived_predicates,
                                              FunctionSkeletonList<Static> static_functions,
                                              FunctionSkeletonList<Fluent> fluent_functions,
                                              std::optional<FunctionSkeleton<Auxiliary>> auxiliary_function,
                                              ActionList actions,
                                              AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(constants.begin(), constants.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_predicates.begin(), static_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_predicates.begin(), fluent_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_functions.begin(), static_functions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_functions.begin(), fluent_functions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(actions.begin(), actions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(axioms.begin(), axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<DomainImpl> {})
        .get_or_create(std::move(filepath),
                       std::move(name),
                       std::move(requirements),
                       std::move(constants),
                       std::move(static_predicates),
                       std::move(fluent_predicates),
                       std::move(derived_predicates),
                       std::move(static_functions),
                       std::move(fluent_functions),
                       std::move(auxiliary_function),
                       std::move(actions),
                       std::move(axioms));
}

Problem PDDLRepositories::get_or_create_problem(std::optional<fs::path> filepath,
                                                Domain domain,
                                                std::string name,
                                                Requirements requirements,
                                                ObjectList objects,
                                                PredicateList<Derived> derived_predicates,
                                                GroundLiteralList<Static> static_initial_literals,
                                                GroundLiteralList<Fluent> fluent_initial_literals,
                                                GroundFunctionValueList<Static> static_function_values,
                                                GroundFunctionValueList<Fluent> fluent_function_values,
                                                std::optional<GroundFunctionValue<Auxiliary>> auxiliary_function_value,
                                                GroundLiteralList<Static> static_goal_condition,
                                                GroundLiteralList<Fluent> fluent_goal_condition,
                                                GroundLiteralList<Derived> derived_goal_condition,
                                                GroundNumericConstraintList numeric_goal_condition,
                                                OptimizationMetric optimization_metric,
                                                AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(objects.begin(), objects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_initial_literals.begin(), static_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_initial_literals.begin(), fluent_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_function_values.begin(), static_function_values.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_function_values.begin(), fluent_function_values.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_goal_condition.begin(), static_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_goal_condition.begin(), fluent_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_goal_condition.begin(), derived_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(numeric_goal_condition.begin(), numeric_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(axioms.begin(), axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ProblemImpl> {})
        .get_or_create(std::move(filepath),
                       std::move(domain),
                       std::move(name),
                       std::move(requirements),
                       std::move(objects),
                       std::move(derived_predicates),
                       std::move(static_initial_literals),
                       std::move(fluent_initial_literals),
                       std::move(static_function_values),
                       std::move(fluent_function_values),
                       std::move(auxiliary_function_value),
                       std::move(static_goal_condition),
                       std::move(fluent_goal_condition),
                       std::move(derived_goal_condition),
                       std::move(numeric_goal_condition),
                       std::move(optimization_metric),
                       std::move(axioms));
}

/**
 * Grounding
 */

// Terms
static void ground_terms(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Object>)
                {
                    out_terms.emplace_back(arg);
                }
                else if constexpr (std::is_same_v<T, Variable>)
                {
                    assert(arg->get_parameter_index() < binding.size());
                    out_terms.emplace_back(binding[arg->get_parameter_index()]);
                }
            },
            term->get_variant());
    }
}
// Literal

template<StaticOrFluentOrDerived P>
GroundLiteral<P> PDDLRepositories::ground(Literal<P> literal, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_literal_grounding_tables, boost::hana::type<GroundLiteral<P>> {});

    /* 2. Access the context-independent literal grounding table */
    const auto is_negated = literal->is_negated();
    const auto predicate_index = literal->get_atom()->get_predicate()->get_index();
    auto& polarity_grounding_tables = grounding_tables[is_negated];
    if (predicate_index >= polarity_grounding_tables.size())
    {
        polarity_grounding_tables.resize(predicate_index + 1);
    }

    auto& grounding_table = polarity_grounding_tables.at(predicate_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    auto grounded_terms = ObjectList {};
    ground_terms(literal->get_atom()->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the literal */

    auto grounded_atom = get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = get_or_create_ground_literal(literal->is_negated(), grounded_atom);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundLiteral<P>(grounded_literal));

    /* 6. Return the resulting ground literal */

    return grounded_literal;
}

template GroundLiteral<Static> PDDLRepositories::ground(Literal<Static> literal, const ObjectList& binding);
template GroundLiteral<Fluent> PDDLRepositories::ground(Literal<Fluent> literal, const ObjectList& binding);
template GroundLiteral<Derived> PDDLRepositories::ground(Literal<Derived> literal, const ObjectList& binding);

template<StaticOrFluentOrDerived P>
void PDDLRepositories::ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                              FlatBitset& ref_positive_bitset,
                                              FlatBitset& ref_negative_bitset,
                                              const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_bitset.set(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_bitset.set(grounded_literal->get_atom()->get_index());
        }
    }
}

template void PDDLRepositories::ground_and_fill_bitset(const std::vector<Literal<Static>>& literals,
                                                       FlatBitset& ref_positive_bitset,
                                                       FlatBitset& ref_negative_bitset,
                                                       const ObjectList& binding);
template void PDDLRepositories::ground_and_fill_bitset(const std::vector<Literal<Fluent>>& literals,
                                                       FlatBitset& ref_positive_bitset,
                                                       FlatBitset& ref_negative_bitset,
                                                       const ObjectList& binding);
template void PDDLRepositories::ground_and_fill_bitset(const std::vector<Literal<Derived>>& literals,
                                                       FlatBitset& ref_positive_bitset,
                                                       FlatBitset& ref_negative_bitset,
                                                       const ObjectList& binding);

template<StaticOrFluentOrDerived P>
void PDDLRepositories::ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                              FlatIndexList& ref_positive_indices,
                                              FlatIndexList& ref_negative_indices,
                                              const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_indices.push_back(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_indices.push_back(grounded_literal->get_atom()->get_index());
        }
    }
    std::sort(ref_positive_indices.begin(), ref_positive_indices.end());
    std::sort(ref_negative_indices.begin(), ref_negative_indices.end());
}

template void PDDLRepositories::ground_and_fill_vector(const std::vector<Literal<Static>>& literals,
                                                       FlatIndexList& ref_positive_indices,
                                                       FlatIndexList& ref_negative_indices,
                                                       const ObjectList& binding);
template void PDDLRepositories::ground_and_fill_vector(const std::vector<Literal<Fluent>>& literals,
                                                       FlatIndexList& ref_positive_indices,
                                                       FlatIndexList& ref_negative_indices,
                                                       const ObjectList& binding);
template void PDDLRepositories::ground_and_fill_vector(const std::vector<Literal<Derived>>& literals,
                                                       FlatIndexList& ref_positive_indices,
                                                       FlatIndexList& ref_negative_indices,
                                                       const ObjectList& binding);

// Function

template<StaticOrFluentOrAuxiliary F>
GroundFunction<F> PDDLRepositories::ground(Function<F> function, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_function_grounding_tables, boost::hana::type<GroundFunction<F>> {});

    /* 2. Access the context-independent function grounding table */
    const auto function_skeleton_index = function->get_function_skeleton()->get_index();
    if (function_skeleton_index >= grounding_tables.size())
    {
        grounding_tables.resize(function_skeleton_index + 1);
    }

    auto& grounding_table = grounding_tables.at(function_skeleton_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    // Note: this is important and saves a lot of memory.
    auto grounded_terms = ObjectList {};
    ground_terms(function->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function */

    const auto grounded_function = get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundFunction<F>(grounded_function));

    /* 6. Return the resulting ground literal */

    return grounded_function;
}

template GroundFunction<Static> PDDLRepositories::ground(Function<Static> function, const ObjectList& binding);
template GroundFunction<Fluent> PDDLRepositories::ground(Function<Fluent> function, const ObjectList& binding);
template GroundFunction<Auxiliary> PDDLRepositories::ground(Function<Auxiliary> function, const ObjectList& binding);

// FunctionExpression

GroundFunctionExpression PDDLRepositories::ground(FunctionExpression fexpr, Problem problem, const ObjectList& binding)
{
    /* 2. Access the context-specific fexpr grounding table
     */

    const auto fexpr_index = fexpr->get_index();
    if (fexpr_index >= m_function_expression_grounding_tables.size())
    {
        m_function_expression_grounding_tables.resize(fexpr_index + 1);
    }

    auto& grounding_table = m_function_expression_grounding_tables.at(fexpr_index);

    /* 3. Check if grounding is cached */

    const auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function expression */
    const auto grounded_fexpr = std::visit(
        [&](auto&& arg) -> GroundFunctionExpression
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>)
            {
                return get_or_create_ground_function_expression(get_or_create_ground_function_expression_number(arg->get_number()));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                const auto op = arg->get_binary_operator();
                const auto ground_lhs = ground(arg->get_left_function_expression(), problem, binding);
                const auto ground_rhs = ground(arg->get_right_function_expression(), problem, binding);

                if (std::holds_alternative<GroundFunctionExpressionNumber>(ground_lhs->get_variant())
                    && std::holds_alternative<GroundFunctionExpressionNumber>(ground_rhs->get_variant()))
                {
                    return get_or_create_ground_function_expression(get_or_create_ground_function_expression_number(
                        evaluate_binary(op,
                                        std::get<GroundFunctionExpressionNumber>(ground_lhs->get_variant())->get_number(),
                                        std::get<GroundFunctionExpressionNumber>(ground_rhs->get_variant())->get_number())));
                }

                return get_or_create_ground_function_expression(get_or_create_ground_function_expression_binary_operator(op, ground_lhs, ground_rhs));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                const auto op = arg->get_multi_operator();
                auto fexpr_numbers = GroundFunctionExpressionList {};
                auto fexpr_others = GroundFunctionExpressionList {};
                for (const auto& child_fexpr : arg->get_function_expressions())
                {
                    const auto ground_child_fexpr = ground(child_fexpr, problem, binding);
                    std::holds_alternative<GroundFunctionExpressionNumber>(ground_child_fexpr->get_variant()) ? fexpr_numbers.push_back(ground_child_fexpr) :
                                                                                                                fexpr_others.push_back(ground_child_fexpr);
                }

                if (!fexpr_numbers.empty())
                {
                    const auto value =
                        std::accumulate(std::next(fexpr_numbers.begin()),  // Start from the second expression
                                        fexpr_numbers.end(),
                                        std::get<GroundFunctionExpressionNumber>(fexpr_numbers.front()->get_variant())->get_number(),  // Initial bounds
                                        [op](const auto& value, const auto& child_expr) {
                                            return evaluate_multi(op, value, std::get<GroundFunctionExpressionNumber>(child_expr->get_variant())->get_number());
                                        });

                    fexpr_others.push_back(get_or_create_ground_function_expression(get_or_create_ground_function_expression_number(value)));
                }

                return get_or_create_ground_function_expression(get_or_create_ground_function_expression_multi_operator(op, fexpr_others));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                const auto ground_fexpr = ground(arg->get_function_expression(), problem, binding);

                return std::holds_alternative<GroundFunctionExpressionNumber>(ground_fexpr->get_variant()) ?
                           get_or_create_ground_function_expression(get_or_create_ground_function_expression_number(
                               -std::get<GroundFunctionExpressionNumber>(ground_fexpr->get_variant())->get_number())) :
                           ground_fexpr;
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Fluent>>)
            {
                return get_or_create_ground_function_expression(
                    get_or_create_ground_function_expression_function<Fluent>(ground(arg->get_function(), binding)));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Static>>)
            {
                return get_or_create_ground_function_expression(
                    get_or_create_ground_function_expression_number(problem->get_function_value<Static>(ground(arg->get_function(), binding))));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "NumericConstraintGrounder::ground(fexpr, binding): Missing implementation for GroundFunctionExpression type.");
            }
        },
        fexpr->get_variant());

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(binding, GroundFunctionExpression(grounded_fexpr));

    /* 6. Return the resulting ground literal */

    return grounded_fexpr;
}

// NumericConstraint
GroundNumericConstraint PDDLRepositories::ground(NumericConstraint numeric_constraint, Problem problem, const ObjectList& binding)
{
    return get_or_create_ground_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                   ground(numeric_constraint->get_left_function_expression(), problem, binding),
                                                   ground(numeric_constraint->get_right_function_expression(), problem, binding));
}

// NumericEffect
template<FluentOrAuxiliary F>
GroundNumericEffect<F> PDDLRepositories::ground(NumericEffect<F> numeric_effect, Problem problem, const ObjectList& binding)
{
    return GroundNumericEffect<F>(numeric_effect->get_assign_operator(),
                                  ground(numeric_effect->get_function(), binding),
                                  ground(numeric_effect->get_function_expression(), problem, binding));
}

template GroundNumericEffect<Fluent> PDDLRepositories::ground(NumericEffect<Fluent> numeric_effect, Problem problem, const ObjectList& binding);
template GroundNumericEffect<Auxiliary> PDDLRepositories::ground(NumericEffect<Auxiliary> numeric_effect, Problem problem, const ObjectList& binding);

// Action

void PDDLRepositories::ground_and_fill_vector(const NumericConstraintList& numeric_constraints,
                                              Problem problem,
                                              const ObjectList& binding,
                                              FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints)
{
    for (const auto& condition : numeric_constraints)
    {
        ref_numeric_constraints.push_back(ground(condition, problem, binding));
    }
}

void PDDLRepositories::ground_and_fill_vector(const NumericEffectList<Fluent>& numeric_effects,
                                              Problem problem,
                                              const ObjectList& binding,
                                              GroundNumericEffectList<Fluent>& ref_numeric_effects)
{
    for (const auto& effect : numeric_effects)
    {
        ref_numeric_effects.push_back(ground(effect, problem, binding));
    }
}

void PDDLRepositories::ground_and_fill_optional(const std::optional<NumericEffect<Auxiliary>>& numeric_effect,
                                                Problem problem,
                                                const ObjectList& binding,
                                                cista::optional<GroundNumericEffect<Auxiliary>>& ref_numeric_effect)
{
    if (numeric_effect.has_value())
    {
        assert(!ref_numeric_effect.has_value());
        ref_numeric_effect = ground(numeric_effect.value(), problem, binding);
    }
}

GroundAction PDDLRepositories::ground(Action action,
                                      Problem problem,
                                      ObjectList binding,
                                      const std::vector<std::vector<IndexList>>& candidate_conditional_effect_objects_by_parameter_index)
{
    /* 1. Check if grounding is cached */

    auto& [action_builder, grounding_table] = m_per_action_datas[action];

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    /* Header */

    action_builder.get_index() = m_actions.size();
    action_builder.get_action_index() = action->get_index();
    auto& objects = action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Conjunctive precondition */
    {
        auto& conjunctive_condition = action_builder.get_conjunctive_condition();
        auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<Fluent>();
        auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<Fluent>();
        auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<Static>();
        auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<Static>();
        auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<Derived>();
        auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<Derived>();
        auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
        positive_fluent_precondition.clear();
        negative_fluent_precondition.clear();
        positive_static_precondition.clear();
        negative_static_precondition.clear();
        positive_derived_precondition.clear();
        negative_derived_precondition.clear();
        numeric_constraints.clear();
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Fluent>(),
                               positive_fluent_precondition,
                               negative_fluent_precondition,
                               binding);
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Static>(),
                               positive_static_precondition,
                               negative_static_precondition,
                               binding);
        ground_and_fill_vector(action->get_conjunctive_condition()->get_literals<Derived>(),
                               positive_derived_precondition,
                               negative_derived_precondition,
                               binding);
        positive_fluent_precondition.compress();
        negative_fluent_precondition.compress();
        positive_static_precondition.compress();
        negative_static_precondition.compress();
        positive_derived_precondition.compress();
        negative_derived_precondition.compress();
        ground_and_fill_vector(action->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, numeric_constraints);

        /* Conjunctive propositional effects */
        auto& conjunctive_effect = action_builder.get_conjunctive_effect();
        auto& positive_effect = conjunctive_effect.get_positive_effects();
        auto& negative_effect = conjunctive_effect.get_negative_effects();
        positive_effect.clear();
        negative_effect.clear();
        ground_and_fill_vector(action->get_conjunctive_effect()->get_literals(), positive_effect, negative_effect, binding);
        positive_effect.compress();
        negative_effect.compress();

        /* Conjunctive numerical effects */
        auto& fluent_numerical_effects = conjunctive_effect.get_fluent_numeric_effects();
        auto& auxiliary_numerical_effect = conjunctive_effect.get_auxiliary_numeric_effect();
        fluent_numerical_effects.clear();
        auxiliary_numerical_effect = std::nullopt;
        ground_and_fill_vector(action->get_conjunctive_effect()->get_fluent_numeric_effects(), problem, binding, fluent_numerical_effects);
        ground_and_fill_optional(action->get_conjunctive_effect()->get_auxiliary_numeric_effect(), problem, binding, auxiliary_numerical_effect);
    }

    /* Conditional effects */
    // Fetch data
    auto& cond_effects = action_builder.get_conditional_effects();

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_cond_effect = binding;

    const auto num_lifted_cond_effects = action->get_conditional_effects().size();
    if (num_lifted_cond_effects > 0)
    {
        size_t j = 0;  ///< position in cond_effects

        for (size_t i = 0; i < num_lifted_cond_effects; ++i)
        {
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& objects_by_parameter_index = candidate_conditional_effect_objects_by_parameter_index.at(i);

                // Resize binding to have additional space for all variables in quantified effect.
                binding_cond_effect.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create resulting conditional effect binding.
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        binding_cond_effect[binding.size() + pos] = get_object(binding_cond[pos]);
                    }

                    auto& cond_effect_j = cond_effects.at(j++);
                    auto& cond_conjunctive_condition_j = cond_effect_j.get_conjunctive_condition();
                    auto& cond_positive_fluent_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Fluent>();
                    auto& cond_negative_fluent_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Fluent>();
                    auto& cond_positive_static_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Static>();
                    auto& cond_negative_static_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Static>();
                    auto& cond_positive_derived_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<Derived>();
                    auto& cond_negative_derived_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<Derived>();
                    auto& cond_numeric_constraints_j = cond_conjunctive_condition_j.get_numeric_constraints();
                    auto& cond_conjunctive_effect_j = cond_effect_j.get_conjunctive_effect();
                    auto& cond_positive_effect_j = cond_conjunctive_effect_j.get_positive_effects();
                    auto& cond_negative_effect_j = cond_conjunctive_effect_j.get_negative_effects();
                    cond_positive_fluent_precondition_j.clear();
                    cond_negative_fluent_precondition_j.clear();
                    cond_positive_static_precondition_j.clear();
                    cond_negative_static_precondition_j.clear();
                    cond_positive_derived_precondition_j.clear();
                    cond_negative_derived_precondition_j.clear();
                    cond_numeric_constraints_j.clear();
                    cond_positive_effect_j.clear();
                    cond_negative_effect_j.clear();

                    /* Propositional precondition */
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Fluent>(),
                                           cond_positive_fluent_precondition_j,
                                           cond_negative_fluent_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Static>(),
                                           cond_positive_static_precondition_j,
                                           cond_negative_static_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Derived>(),
                                           cond_positive_derived_precondition_j,
                                           cond_negative_derived_precondition_j,
                                           binding_cond_effect);
                    cond_positive_fluent_precondition_j.compress();
                    cond_negative_fluent_precondition_j.compress();
                    cond_positive_static_precondition_j.compress();
                    cond_negative_static_precondition_j.compress();
                    cond_positive_derived_precondition_j.compress();
                    cond_negative_derived_precondition_j.compress();
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(),
                                           problem,
                                           binding_cond_effect,
                                           cond_numeric_constraints_j);

                    /* Propositional effect */
                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_literals(),
                                           cond_positive_effect_j,
                                           cond_negative_effect_j,
                                           binding_cond_effect);

                    /* Numeric effect */
                    auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect_j.get_fluent_numeric_effects();
                    auto& cond_auxiliary_numerical_effect_j = cond_conjunctive_effect_j.get_auxiliary_numeric_effect();
                    cond_fluent_numerical_effects_j.clear();
                    cond_auxiliary_numerical_effect_j = std::nullopt;

                    ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                           problem,
                                           binding_cond_effect,
                                           cond_fluent_numerical_effects_j);
                    ground_and_fill_optional(lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                             problem,
                                             binding_cond_effect,
                                             cond_auxiliary_numerical_effect_j);
                }
            }
            else
            {
                auto& cond_effect = cond_effects.at(j++);
                auto& cond_conjunctive_condition = cond_effect.get_conjunctive_condition();
                auto& cond_positive_fluent_precondition = cond_conjunctive_condition.get_positive_precondition<Fluent>();
                auto& cond_negative_fluent_precondition = cond_conjunctive_condition.get_negative_precondition<Fluent>();
                auto& cond_positive_static_precondition = cond_conjunctive_condition.get_positive_precondition<Static>();
                auto& cond_negative_static_precondition = cond_conjunctive_condition.get_negative_precondition<Static>();
                auto& cond_positive_derived_precondition = cond_conjunctive_condition.get_positive_precondition<Derived>();
                auto& cond_negative_derived_precondition = cond_conjunctive_condition.get_negative_precondition<Derived>();
                auto& cond_numeric_constraints = cond_conjunctive_condition.get_numeric_constraints();
                auto& cond_conjunctive_effect = cond_effect.get_conjunctive_effect();
                auto& cond_positive_effect = cond_conjunctive_effect.get_positive_effects();
                auto& cond_negative_effect = cond_conjunctive_effect.get_negative_effects();
                cond_positive_fluent_precondition.clear();
                cond_negative_fluent_precondition.clear();
                cond_positive_static_precondition.clear();
                cond_negative_static_precondition.clear();
                cond_positive_derived_precondition.clear();
                cond_negative_derived_precondition.clear();
                cond_numeric_constraints.clear();
                cond_positive_effect.clear();
                cond_negative_effect.clear();

                /* Propositional precondition */
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Fluent>(),
                                       cond_positive_fluent_precondition,
                                       cond_negative_fluent_precondition,
                                       binding);
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Static>(),
                                       cond_positive_static_precondition,
                                       cond_negative_static_precondition,
                                       binding);
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_literals<Derived>(),
                                       cond_positive_derived_precondition,
                                       cond_negative_derived_precondition,
                                       binding);
                cond_positive_fluent_precondition.compress();
                cond_negative_fluent_precondition.compress();
                cond_positive_static_precondition.compress();
                cond_negative_static_precondition.compress();
                cond_positive_derived_precondition.compress();
                cond_negative_derived_precondition.compress();
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, cond_numeric_constraints);

                /* Propositional effect */
                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_literals(), cond_positive_effect, cond_negative_effect, binding);

                /* Numeric effect*/
                auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect.get_fluent_numeric_effects();
                auto& cond_auxiliary_numerical_effects_j = cond_conjunctive_effect.get_auxiliary_numeric_effect();
                cond_fluent_numerical_effects_j.clear();
                cond_auxiliary_numerical_effects_j = std::nullopt;

                ground_and_fill_vector(lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                       problem,
                                       binding,
                                       cond_fluent_numerical_effects_j);
                ground_and_fill_optional(lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                         problem,
                                         binding,
                                         cond_auxiliary_numerical_effects_j);
            }
        }
    }

    const auto [iter, inserted] = m_actions.insert(action_builder);
    const auto grounded_action = iter->get();
    if (inserted)
    {
        m_actions_by_index.push_back(grounded_action);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

const GroundActionList& PDDLRepositories::get_ground_actions() const { return m_actions_by_index; }

GroundAction PDDLRepositories::get_ground_action(Index action_index) const { return m_actions_by_index.at(action_index); }

size_t PDDLRepositories::get_num_ground_actions() const { return m_actions_by_index.size(); }

size_t PDDLRepositories::get_estimated_memory_usage_in_bytes_for_actions() const
{
    const auto usage1 = m_actions.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_actions_by_index.capacity() * sizeof(GroundAction);
    auto usage3 = size_t(0);

    for (const auto& [action, action_data] : m_per_action_datas)
    {
        const auto& [action_builder, grounding_table] = action_data;

        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}

// Axiom

GroundAxiom PDDLRepositories::ground(Axiom axiom, Problem problem, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& [axiom_builder, grounding_table] = m_per_axiom_data[axiom];
    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    /* Header */

    axiom_builder.get_index() = m_axioms.size();
    axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = axiom_builder.get_object_indices();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Precondition */
    auto& conjunctive_condition = axiom_builder.get_conjunctive_condition();
    auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<Static>();
    auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<Derived>();
    auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    numeric_constraints.clear();
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Static>(), positive_static_precondition, negative_static_precondition, binding);
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_literals<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);
    positive_fluent_precondition.compress();
    negative_fluent_precondition.compress();
    positive_static_precondition.compress();
    negative_static_precondition.compress();
    positive_derived_precondition.compress();
    negative_derived_precondition.compress();
    ground_and_fill_vector(axiom->get_conjunctive_condition()->get_numeric_constraints(), problem, binding, numeric_constraints);

    /* Effect */

    // The effect literal might only use the first few objects of the complete binding
    // Therefore, we can prevent the literal grounding table from unnecessarily growing
    // by restricting the binding to only the relevant part
    const auto effect_literal_arity = axiom->get_literal()->get_atom()->get_arity();
    const auto is_complete_binding_relevant_for_head = (binding.size() == effect_literal_arity);
    const auto grounded_literal = is_complete_binding_relevant_for_head ?
                                      ground(axiom->get_literal(), binding) :
                                      ground(axiom->get_literal(), ObjectList(binding.begin(), binding.begin() + effect_literal_arity));
    assert(!grounded_literal->is_negated());
    axiom_builder.get_derived_effect().is_negated = false;
    axiom_builder.get_derived_effect().atom_index = grounded_literal->get_atom()->get_index();

    const auto [iter, inserted] = m_axioms.insert(axiom_builder);
    const auto grounded_axiom = iter->get();

    if (inserted)
    {
        m_axioms_by_index.push_back(grounded_axiom);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

const GroundAxiomList& PDDLRepositories::get_ground_axioms() const { return m_axioms_by_index; }

GroundAxiom PDDLRepositories::get_ground_axiom(Index axiom_index) const { return m_axioms_by_index.at(axiom_index); }

size_t PDDLRepositories::get_num_ground_axioms() const { return m_axioms_by_index.size(); }

size_t PDDLRepositories::get_estimated_memory_usage_in_bytes_for_axioms() const
{
    const auto usage1 = m_axioms.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_axioms_by_index.capacity() * sizeof(GroundAxiom);
    auto usage3 = size_t(0);
    // TODO: add memory usage of m_per_axiom_data
    for (const auto& [axiom, action_data] : m_per_axiom_data)
    {
        const auto& [axiom_builder, grounding_table] = action_data;
        // TODO: add memory usage of axiom_builder
        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}

/**
 * Accessors
 */

// Factory
const PDDLTypeToRepository& PDDLRepositories::get_pddl_type_to_factory() const { return m_repositories; }

// GroundAtom
template<StaticOrFluentOrDerived P>
GroundAtom<P> PDDLRepositories::get_ground_atom(size_t atom_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).at(atom_index);
}

template GroundAtom<Static> PDDLRepositories::get_ground_atom<Static>(size_t atom_index) const;
template GroundAtom<Fluent> PDDLRepositories::get_ground_atom<Fluent>(size_t atom_index) const;
template GroundAtom<Derived> PDDLRepositories::get_ground_atom<Derived>(size_t atom_index) const;

// GroundFunction

template<StaticOrFluentOrAuxiliary F>
void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<F>& out_ground_functions)
{
    out_ground_functions.clear();

    const auto& ground_functions = boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {});

    out_ground_functions.insert(out_ground_functions.end(), ground_functions.begin(), ground_functions.begin() + num_ground_functions);
}

template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Static>& out_ground_functions);
template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Fluent>& out_ground_functions);
template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Auxiliary>& out_ground_functions);

template<StaticOrFluentOrAuxiliary F>
void PDDLRepositories::get_ground_function_values(const FlatDoubleList& values,
                                                  std::vector<std::pair<GroundFunction<F>, ContinuousCost>>& out_ground_function_values) const
{
    out_ground_function_values.clear();

    const auto& ground_functions = boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {});

    for (size_t i = 0; i < values.size(); ++i)
    {
        out_ground_function_values.emplace_back(ground_functions.at(i), values[i]);
    }
}

template void PDDLRepositories::get_ground_function_values(const FlatDoubleList& values,
                                                           std::vector<std::pair<GroundFunction<Static>, ContinuousCost>>& out_ground_function_values) const;
template void PDDLRepositories::get_ground_function_values(const FlatDoubleList& values,
                                                           std::vector<std::pair<GroundFunction<Fluent>, ContinuousCost>>& out_ground_function_values) const;
template void PDDLRepositories::get_ground_function_values(const FlatDoubleList& values,
                                                           std::vector<std::pair<GroundFunction<Auxiliary>, ContinuousCost>>& out_ground_function_values) const;

template<StaticOrFluentOrAuxiliary F>
std::vector<std::pair<GroundFunction<F>, ContinuousCost>> PDDLRepositories::get_ground_function_values(const FlatDoubleList& values) const
{
    auto ground_function_values = std::vector<std::pair<GroundFunction<F>, ContinuousCost>> {};

    get_ground_function_values(values, ground_function_values);

    return ground_function_values;
}

template std::vector<std::pair<GroundFunction<Static>, ContinuousCost>> PDDLRepositories::get_ground_function_values(const FlatDoubleList& values) const;
template std::vector<std::pair<GroundFunction<Fluent>, ContinuousCost>> PDDLRepositories::get_ground_function_values(const FlatDoubleList& values) const;
template std::vector<std::pair<GroundFunction<Auxiliary>, ContinuousCost>> PDDLRepositories::get_ground_function_values(const FlatDoubleList& values) const;

// Object
Object PDDLRepositories::get_object(size_t object_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).at(object_index);
}

// Action
Action PDDLRepositories::get_action(size_t action_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {}).at(action_index);
}

// Axiom
Axiom PDDLRepositories::get_axiom(size_t axiom_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).at(axiom_index); }

}
