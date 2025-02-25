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

HanaPDDLRepositories& PDDLRepositories::get_hana_repositories() { return m_repositories; }

const HanaPDDLRepositories& PDDLRepositories::get_hana_repositories() const { return m_repositories; }

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

template<StaticOrFluent F>
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

template<StaticOrFluent F>
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

template<StaticOrFluentOrAuxiliary F>
GroundFunctionExpressionFunction<F> PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction<F> function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionFunctionImpl<F>> {}).get_or_create(std::move(function));
}

template GroundFunctionExpressionFunction<Static> PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction<Static> function);
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

template<StaticOrFluentOrAuxiliary F>
GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<F> fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}
template GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<Static> fexpr);
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

template<FluentOrAuxiliary F>
GroundNumericEffect<F> PDDLRepositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                             GroundFunction<F> function,
                                                                             GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericEffectImpl<F>> {})
        .get_or_create(std::move(assign_operator), std::move(function), std::move(function_expression));
}

template GroundNumericEffect<Fluent> PDDLRepositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                                           GroundFunction<Fluent> function,
                                                                                           GroundFunctionExpression function_expression);
template GroundNumericEffect<Auxiliary> PDDLRepositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                                              GroundFunction<Auxiliary> function,
                                                                                              GroundFunctionExpression function_expression);

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
                                                                           LiteralLists<Static, Fluent, Derived> literals,
                                                                           NumericConstraintList numeric_constraints)
{
    auto nullary_ground_literals = GroundLiteralLists<Static, Fluent, Derived> {};
    boost::hana::for_each(literals,
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              auto& key_literals = boost::hana::second(pair);

                              /* Canonize before uniqueness test */
                              std::sort(key_literals.begin(), key_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

                              auto& key_nullary_ground_literals = boost::hana::at_key(nullary_ground_literals, key);
                              key_nullary_ground_literals = ground_nullary_literals(key_literals, *this);

                              std::sort(key_nullary_ground_literals.begin(),
                                        key_nullary_ground_literals.end(),
                                        [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
                          });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveConditionImpl> {})
        .get_or_create(std::move(parameters), std::move(literals), std::move(nullary_ground_literals), std::move(numeric_constraints));
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

/**
 * Accessors
 */

// GroundNumericConstraint

GroundNumericConstraint PDDLRepositories::get_ground_numeric_constraint(size_t numeric_constraint_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericConstraintImpl> {}).at(numeric_constraint_index);
}

// GroundNumericEffect

template<FluentOrAuxiliary F>
GroundNumericEffect<F> PDDLRepositories::get_ground_numeric_effect(size_t numeric_effect_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericEffectImpl<F>> {}).at(numeric_effect_index);
}

template GroundNumericEffect<Fluent> PDDLRepositories::get_ground_numeric_effect(size_t numeric_effect_index) const;
template GroundNumericEffect<Auxiliary> PDDLRepositories::get_ground_numeric_effect(size_t numeric_effect_index) const;

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
void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<F>& out_ground_functions) const
{
    out_ground_functions.clear();

    const auto& ground_functions = boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {});

    out_ground_functions.insert(out_ground_functions.end(), ground_functions.begin(), ground_functions.begin() + num_ground_functions);
}

template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Static>& out_ground_functions) const;
template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Fluent>& out_ground_functions) const;
template void PDDLRepositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<Auxiliary>& out_ground_functions) const;

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
