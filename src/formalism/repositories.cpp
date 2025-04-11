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

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
static GroundLiteralList<P> ground_nullary_literals(const LiteralList<P>& literals, Repositories& pddl_repositories)
{
    auto ground_literals = GroundLiteralList<P> {};
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_arity() != 0)
            continue;

        ground_literals.push_back(
            pddl_repositories.get_or_create_ground_literal(literal->get_polarity(),
                                                           pddl_repositories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), {})));
    }

    return ground_literals;
}

HanaRepositories& Repositories::get_hana_repositories() { return m_repositories; }

const HanaRepositories& Repositories::get_hana_repositories() const { return m_repositories; }

Requirements Repositories::get_or_create_requirements(loki::RequirementEnumSet requirement_set)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RequirementsImpl> {}).get_or_create(std::move(requirement_set));
}

Variable Repositories::get_or_create_variable(std::string name, size_t parameter_index)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<VariableImpl> {}).get_or_create(std::move(name), std::move(parameter_index));
}

Term Repositories::get_or_create_term(Variable variable)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(variable));
}

Term Repositories::get_or_create_term(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(object));
}

Object Repositories::get_or_create_object(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).get_or_create(std::move(name));
}

template<IsStaticOrFluentOrDerivedTag P>
Atom<P> Repositories::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AtomImpl<P>> {}).get_or_create(predicate, std::move(terms));
}

template Atom<StaticTag> Repositories::get_or_create_atom(Predicate<StaticTag> predicate, TermList terms);
template Atom<FluentTag> Repositories::get_or_create_atom(Predicate<FluentTag> predicate, TermList terms);
template Atom<DerivedTag> Repositories::get_or_create_atom(Predicate<DerivedTag> predicate, TermList terms);

template<IsStaticOrFluentOrDerivedTag P>
GroundAtom<P> Repositories::get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).get_or_create(predicate, std::move(objects));
}

template GroundAtom<StaticTag> Repositories::get_or_create_ground_atom(Predicate<StaticTag> predicate, ObjectList ObjectList);
template GroundAtom<FluentTag> Repositories::get_or_create_ground_atom(Predicate<FluentTag> predicate, ObjectList ObjectList);
template GroundAtom<DerivedTag> Repositories::get_or_create_ground_atom(Predicate<DerivedTag> predicate, ObjectList ObjectList);

template<IsStaticOrFluentOrDerivedTag P>
Literal<P> Repositories::get_or_create_literal(bool polarity, Atom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<LiteralImpl<P>> {}).get_or_create(polarity, atom);
}

template Literal<StaticTag> Repositories::get_or_create_literal(bool polarity, Atom<StaticTag> atom);
template Literal<FluentTag> Repositories::get_or_create_literal(bool polarity, Atom<FluentTag> atom);
template Literal<DerivedTag> Repositories::get_or_create_literal(bool polarity, Atom<DerivedTag> atom);

template<IsStaticOrFluentOrDerivedTag P>
GroundLiteral<P> Repositories::get_or_create_ground_literal(bool polarity, GroundAtom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundLiteralImpl<P>> {}).get_or_create(polarity, atom);
}

template GroundLiteral<StaticTag> Repositories::get_or_create_ground_literal(bool polarity, GroundAtom<StaticTag> atom);
template GroundLiteral<FluentTag> Repositories::get_or_create_ground_literal(bool polarity, GroundAtom<FluentTag> atom);
template GroundLiteral<DerivedTag> Repositories::get_or_create_ground_literal(bool polarity, GroundAtom<DerivedTag> atom);

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> Repositories::get_or_create_predicate(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PredicateImpl<P>> {}).get_or_create(name, std::move(parameters));
}

template Predicate<StaticTag> Repositories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<FluentTag> Repositories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<DerivedTag> Repositories::get_or_create_predicate(std::string name, VariableList parameters);

FunctionExpressionNumber Repositories::get_or_create_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionNumberImpl> {}).get_or_create(number);
}

FunctionExpressionBinaryOperator Repositories::get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
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
        .get_or_create(binary_operator, left_function_expression, right_function_expression);
}

FunctionExpressionMultiOperator Repositories::get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                               FunctionExpressionList function_expressions)
{
    /* Canonize before uniqueness test. */
    std::sort(function_expressions.begin(), function_expressions.end(), [](auto&& l, auto&& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionMultiOperatorImpl> {})
        .get_or_create(multi_operator, std::move(function_expressions));
}

FunctionExpressionMinus Repositories::get_or_create_function_expression_minus(FunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionMinusImpl> {}).get_or_create(function_expression);
}

template<IsStaticOrFluentTag F>
FunctionExpressionFunction<F> Repositories::get_or_create_function_expression_function(Function<F> function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionFunctionImpl<F>> {}).get_or_create(std::move(function));
}

template FunctionExpressionFunction<StaticTag> Repositories::get_or_create_function_expression_function(Function<StaticTag> function);
template FunctionExpressionFunction<FluentTag> Repositories::get_or_create_function_expression_function(Function<FluentTag> function);

FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionNumber fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionMultiOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionMinus fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

template<IsStaticOrFluentTag F>
FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionFunction<F> fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

template FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionFunction<StaticTag> fexpr);
template FunctionExpression Repositories::get_or_create_function_expression(FunctionExpressionFunction<FluentTag> fexpr);

GroundFunctionExpressionNumber Repositories::get_or_create_ground_function_expression_number(double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionNumberImpl> {}).get_or_create(number);
}

GroundFunctionExpressionBinaryOperator
Repositories::get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                       GroundFunctionExpression left_function_expression,
                                                                       GroundFunctionExpression right_function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionBinaryOperatorImpl> {})
        .get_or_create(binary_operator, std::move(left_function_expression), std::move(right_function_expression));
}

GroundFunctionExpressionMultiOperator Repositories::get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
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

GroundFunctionExpressionMinus Repositories::get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionMinusImpl> {}).get_or_create(std::move(function_expression));
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionExpressionFunction<F> Repositories::get_or_create_ground_function_expression_function(GroundFunction<F> function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionFunctionImpl<F>> {}).get_or_create(std::move(function));
}

template GroundFunctionExpressionFunction<StaticTag> Repositories::get_or_create_ground_function_expression_function(GroundFunction<StaticTag> function);
template GroundFunctionExpressionFunction<FluentTag> Repositories::get_or_create_ground_function_expression_function(GroundFunction<FluentTag> function);
template GroundFunctionExpressionFunction<AuxiliaryTag> Repositories::get_or_create_ground_function_expression_function(GroundFunction<AuxiliaryTag> function);

GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionNumber fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionBinaryOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionMultiOperator fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionMinus fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<F> fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}
template GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<StaticTag> fexpr);
template GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<FluentTag> fexpr);
template GroundFunctionExpression Repositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction<AuxiliaryTag> fexpr);

template<IsStaticOrFluentOrAuxiliaryTag F>
Function<F> Repositories::get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionImpl<F>> {})
        .get_or_create(std::move(function_skeleton), std::move(terms), std::move(m_parent_terms_to_terms_mapping));
}

template Function<StaticTag>
Repositories::get_or_create_function(FunctionSkeleton<StaticTag> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);
template Function<FluentTag>
Repositories::get_or_create_function(FunctionSkeleton<FluentTag> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);
template Function<AuxiliaryTag>
Repositories::get_or_create_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, TermList terms, IndexList m_parent_terms_to_terms_mapping);

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> Repositories::get_or_create_ground_function(FunctionSkeleton<F> function_skeleton, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {}).get_or_create(std::move(function_skeleton), std::move(objects));
}

template GroundFunction<StaticTag> Repositories::get_or_create_ground_function(FunctionSkeleton<StaticTag> function_skeleton, ObjectList objects);
template GroundFunction<FluentTag> Repositories::get_or_create_ground_function(FunctionSkeleton<FluentTag> function_skeleton, ObjectList objects);
template GroundFunction<AuxiliaryTag> Repositories::get_or_create_ground_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, ObjectList objects);

template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeleton<F> Repositories::get_or_create_function_skeleton(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionSkeletonImpl<F>> {}).get_or_create(std::move(name), std::move(parameters));
}

template FunctionSkeleton<StaticTag> Repositories::get_or_create_function_skeleton(std::string name, VariableList parameters);
template FunctionSkeleton<FluentTag> Repositories::get_or_create_function_skeleton(std::string name, VariableList parameters);
template FunctionSkeleton<AuxiliaryTag> Repositories::get_or_create_function_skeleton(std::string name, VariableList parameters);

template<IsFluentOrAuxiliaryTag F>
NumericEffect<F>
Repositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<F> function, FunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericEffectImpl<F>> {})
        .get_or_create(std::move(assign_operator), std::move(function), std::move(function_expression));
}

template NumericEffect<FluentTag>
Repositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<FluentTag> function, FunctionExpression function_expression);
template NumericEffect<AuxiliaryTag>
Repositories::get_or_create_numeric_effect(loki::AssignOperatorEnum assign_operator, Function<AuxiliaryTag> function, FunctionExpression function_expression);

template<IsFluentOrAuxiliaryTag F>
GroundNumericEffect<F> Repositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                         GroundFunction<F> function,
                                                                         GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericEffectImpl<F>> {})
        .get_or_create(std::move(assign_operator), std::move(function), std::move(function_expression));
}

template GroundNumericEffect<FluentTag> Repositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                                          GroundFunction<FluentTag> function,
                                                                                          GroundFunctionExpression function_expression);
template GroundNumericEffect<AuxiliaryTag> Repositories::get_or_create_ground_numeric_effect(loki::AssignOperatorEnum assign_operator,
                                                                                             GroundFunction<AuxiliaryTag> function,
                                                                                             GroundFunctionExpression function_expression);

ConjunctiveEffect Repositories::get_or_create_conjunctive_effect(VariableList parameters,
                                                                 LiteralList<FluentTag> effects,
                                                                 NumericEffectList<FluentTag> fluent_numeric_effects,
                                                                 std::optional<NumericEffect<AuxiliaryTag>> auxiliary_numeric_effect)
{
    /* Canonize before uniqueness test. */
    std::sort(effects.begin(), effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_numeric_effects.begin(), fluent_numeric_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveEffectImpl> {})
        .get_or_create(std::move(parameters), std::move(effects), std::move(fluent_numeric_effects), std::move(auxiliary_numeric_effect));
}

ConditionalEffect Repositories::get_or_create_conditional_effect(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConditionalEffectImpl> {}).get_or_create(conjunctive_condition, conjunctive_effect);
}

NumericConstraint Repositories::get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                 FunctionExpression left_function_expression,
                                                                 FunctionExpression right_function_expression,
                                                                 TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericConstraintImpl> {})
        .get_or_create(binary_comparator, left_function_expression, right_function_expression, std::move(terms));
}

GroundNumericConstraint Repositories::get_or_create_ground_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                              GroundFunctionExpression left_function_expression,
                                                                              GroundFunctionExpression right_function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericConstraintImpl> {})
        .get_or_create(binary_comparator, left_function_expression, right_function_expression);
}

ConjunctiveCondition Repositories::get_or_create_conjunctive_condition(VariableList parameters,
                                                                       LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                                       NumericConstraintList numeric_constraints)
{
    auto nullary_ground_literals = GroundLiteralLists<StaticTag, FluentTag, DerivedTag> {};
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

    std::sort(numeric_constraints.begin(), numeric_constraints.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveConditionImpl> {})
        .get_or_create(std::move(parameters), std::move(literals), std::move(nullary_ground_literals), std::move(numeric_constraints));
}

Action Repositories::get_or_create_action(std::string name,
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

Axiom Repositories::get_or_create_axiom(ConjunctiveCondition conjunctive_condition, Literal<DerivedTag> literal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).get_or_create(std::move(conjunctive_condition), std::move(literal));
}

OptimizationMetric Repositories::get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<OptimizationMetricImpl> {}).get_or_create(std::move(metric), std::move(function_expression));
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunctionValue<F> Repositories::get_or_create_ground_function_value(GroundFunction<F> function, double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionValueImpl<F>> {}).get_or_create(std::move(function), std::move(number));
}

template GroundFunctionValue<StaticTag> Repositories::get_or_create_ground_function_value(GroundFunction<StaticTag> function, double number);
template GroundFunctionValue<FluentTag> Repositories::get_or_create_ground_function_value(GroundFunction<FluentTag> function, double number);
template GroundFunctionValue<AuxiliaryTag> Repositories::get_or_create_ground_function_value(GroundFunction<AuxiliaryTag> function, double number);

/**
 * Accessors
 */

// GroundNumericConstraint

GroundNumericConstraint Repositories::get_ground_numeric_constraint(size_t numeric_constraint_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericConstraintImpl> {}).at(numeric_constraint_index);
}

// GroundNumericEffect

template<IsFluentOrAuxiliaryTag F>
GroundNumericEffect<F> Repositories::get_ground_numeric_effect(size_t numeric_effect_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundNumericEffectImpl<F>> {}).at(numeric_effect_index);
}

template GroundNumericEffect<FluentTag> Repositories::get_ground_numeric_effect(size_t numeric_effect_index) const;
template GroundNumericEffect<AuxiliaryTag> Repositories::get_ground_numeric_effect(size_t numeric_effect_index) const;

// GroundAtom
template<IsStaticOrFluentOrDerivedTag P>
GroundAtom<P> Repositories::get_ground_atom(size_t atom_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).at(atom_index);
}

template GroundAtom<StaticTag> Repositories::get_ground_atom<StaticTag>(size_t atom_index) const;
template GroundAtom<FluentTag> Repositories::get_ground_atom<FluentTag>(size_t atom_index) const;
template GroundAtom<DerivedTag> Repositories::get_ground_atom<DerivedTag>(size_t atom_index) const;

// GroundFunction

template<IsStaticOrFluentOrAuxiliaryTag F>
void Repositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<F>& out_ground_functions) const
{
    out_ground_functions.clear();

    const auto& ground_functions = boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {});

    out_ground_functions.insert(out_ground_functions.end(), ground_functions.begin(), ground_functions.begin() + num_ground_functions);
}

template void Repositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<StaticTag>& out_ground_functions) const;
template void Repositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<FluentTag>& out_ground_functions) const;
template void Repositories::get_ground_functions(size_t num_ground_functions, GroundFunctionList<AuxiliaryTag>& out_ground_functions) const;

template<IsStaticOrFluentOrAuxiliaryTag F>
void Repositories::get_ground_function_values(const FlatDoubleList& values,
                                              std::vector<std::pair<GroundFunction<F>, ContinuousCost>>& out_ground_function_values) const
{
    out_ground_function_values.clear();

    const auto& ground_functions = boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {});

    for (size_t i = 0; i < values.size(); ++i)
    {
        out_ground_function_values.emplace_back(ground_functions.at(i), values[i]);
    }
}

template void Repositories::get_ground_function_values(const FlatDoubleList& values,
                                                       std::vector<std::pair<GroundFunction<StaticTag>, ContinuousCost>>& out_ground_function_values) const;
template void Repositories::get_ground_function_values(const FlatDoubleList& values,
                                                       std::vector<std::pair<GroundFunction<FluentTag>, ContinuousCost>>& out_ground_function_values) const;
template void Repositories::get_ground_function_values(const FlatDoubleList& values,
                                                       std::vector<std::pair<GroundFunction<AuxiliaryTag>, ContinuousCost>>& out_ground_function_values) const;

template<IsStaticOrFluentOrAuxiliaryTag F>
std::vector<std::pair<GroundFunction<F>, ContinuousCost>> Repositories::get_ground_function_values(const FlatDoubleList& values) const
{
    auto ground_function_values = std::vector<std::pair<GroundFunction<F>, ContinuousCost>> {};

    get_ground_function_values(values, ground_function_values);

    return ground_function_values;
}

template std::vector<std::pair<GroundFunction<StaticTag>, ContinuousCost>> Repositories::get_ground_function_values(const FlatDoubleList& values) const;
template std::vector<std::pair<GroundFunction<FluentTag>, ContinuousCost>> Repositories::get_ground_function_values(const FlatDoubleList& values) const;
template std::vector<std::pair<GroundFunction<AuxiliaryTag>, ContinuousCost>> Repositories::get_ground_function_values(const FlatDoubleList& values) const;

// Object
Object Repositories::get_object(size_t object_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).at(object_index); }

// Action
Action Repositories::get_action(size_t action_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {}).at(action_index); }

// Axiom
Axiom Repositories::get_axiom(size_t axiom_index) const { return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).at(axiom_index); }

}
