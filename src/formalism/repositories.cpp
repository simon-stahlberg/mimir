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

namespace mimir
{

PDDLTypeToRepository create_default_pddl_type_to_repository()
{
    return boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type_c<RequirementsImpl>, RequirementsRepository {}),
        boost::hana::make_pair(boost::hana::type_c<VariableImpl>, VariableRepository {}),
        boost::hana::make_pair(boost::hana::type_c<TermImpl>, TermRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ObjectImpl>, ObjectRepository {}),
        boost::hana::make_pair(boost::hana::type_c<AtomImpl<Static>>, AtomRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<AtomImpl<Fluent>>, AtomRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<AtomImpl<Derived>>, AtomRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Static>>, GroundAtomRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Fluent>>, GroundAtomRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundAtomImpl<Derived>>, GroundAtomRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Static>>, LiteralRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Fluent>>, LiteralRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<LiteralImpl<Derived>>, LiteralRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Static>>, GroundLiteralRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Fluent>>, GroundLiteralRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<GroundLiteralImpl<Derived>>, GroundLiteralRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Static>>, PredicateRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Fluent>>, PredicateRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<PredicateImpl<Derived>>, PredicateRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionNumberImpl>, FunctionExpressionNumberRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionBinaryOperatorImpl>, FunctionExpressionBinaryOperatorRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionMultiOperatorImpl>, FunctionExpressionMultiOperatorRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionMinusImpl>, FunctionExpressionMinusRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionFunctionImpl>, FunctionExpressionFunctionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionExpressionImpl>, FunctionExpressionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionNumberImpl>, GroundFunctionExpressionNumberRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionBinaryOperatorImpl>, GroundFunctionExpressionBinaryOperatorRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionMultiOperatorImpl>, GroundFunctionExpressionMultiOperatorRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionMinusImpl>, GroundFunctionExpressionMinusRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionFunctionImpl>, GroundFunctionExpressionFunctionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionExpressionImpl>, GroundFunctionExpressionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionImpl>, FunctionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<GroundFunctionImpl>, GroundFunctionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<FunctionSkeletonImpl>, FunctionSkeletonRepository {}),
        boost::hana::make_pair(boost::hana::type_c<EffectStripsImpl>, EffectStripsRepository {}),
        boost::hana::make_pair(boost::hana::type_c<EffectConditionalImpl>, EffectUniversalRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ExistentiallyQuantifiedConjunctiveConditionImpl>, UniversallyQuantifiedConjunctionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ActionImpl>, ActionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<AxiomImpl>, AxiomRepository {}),
        boost::hana::make_pair(boost::hana::type_c<OptimizationMetricImpl>, OptimizationMetricRepository {}),
        boost::hana::make_pair(boost::hana::type_c<NumericFluentImpl>, NumericFluentRepository {}),
        boost::hana::make_pair(boost::hana::type_c<DomainImpl>, DomainRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ProblemImpl>, ProblemRepository {}));
}

template<PredicateTag P>
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

PDDLRepositories::PDDLRepositories() : m_repositories(create_default_pddl_type_to_repository()) {}

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

template<PredicateTag P>
Atom<P> PDDLRepositories::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AtomImpl<P>> {}).get_or_create(std::move(predicate), std::move(terms));
}

template Atom<Static> PDDLRepositories::get_or_create_atom(Predicate<Static> predicate, TermList terms);
template Atom<Fluent> PDDLRepositories::get_or_create_atom(Predicate<Fluent> predicate, TermList terms);
template Atom<Derived> PDDLRepositories::get_or_create_atom(Predicate<Derived> predicate, TermList terms);

template<PredicateTag P>
GroundAtom<P> PDDLRepositories::get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).get_or_create(std::move(predicate), std::move(objects));
}

template GroundAtom<Static> PDDLRepositories::get_or_create_ground_atom(Predicate<Static> predicate, ObjectList ObjectList);
template GroundAtom<Fluent> PDDLRepositories::get_or_create_ground_atom(Predicate<Fluent> predicate, ObjectList ObjectList);
template GroundAtom<Derived> PDDLRepositories::get_or_create_ground_atom(Predicate<Derived> predicate, ObjectList ObjectList);

template<PredicateTag P>
Literal<P> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<LiteralImpl<P>> {}).get_or_create(is_negated, std::move(atom));
}

template Literal<Static> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Static> atom);
template Literal<Fluent> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Fluent> atom);
template Literal<Derived> PDDLRepositories::get_or_create_literal(bool is_negated, Atom<Derived> atom);

template<PredicateTag P>
GroundLiteral<P> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundLiteralImpl<P>> {}).get_or_create(is_negated, std::move(atom));
}

template GroundLiteral<Static> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Static> atom);
template GroundLiteral<Fluent> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Fluent> atom);
template GroundLiteral<Derived> PDDLRepositories::get_or_create_ground_literal(bool is_negated, GroundAtom<Derived> atom);

template<PredicateTag P>
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

FunctionExpressionFunction PDDLRepositories::get_or_create_function_expression_function(Function function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionFunctionImpl> {}).get_or_create(std::move(function));
}

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

FunctionExpression PDDLRepositories::get_or_create_function_expression(FunctionExpressionFunction fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionExpressionImpl> {}).get_or_create(fexpr);
}

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

GroundFunctionExpressionFunction PDDLRepositories::get_or_create_ground_function_expression_function(GroundFunction function)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionFunctionImpl> {}).get_or_create(std::move(function));
}

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

GroundFunctionExpression PDDLRepositories::get_or_create_ground_function_expression(GroundFunctionExpressionFunction fexpr)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionExpressionImpl> {}).get_or_create(fexpr);
}

Function PDDLRepositories::get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionImpl> {}).get_or_create(std::move(function_skeleton), std::move(terms));
}

GroundFunction PDDLRepositories::get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl> {}).get_or_create(std::move(function_skeleton), std::move(objects));
}

FunctionSkeleton PDDLRepositories::get_or_create_function_skeleton(std::string name, VariableList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionSkeletonImpl> {}).get_or_create(std::move(name), std::move(parameters));
}

EffectStrips PDDLRepositories::get_or_create_strips_effect(LiteralList<Fluent> effects, FunctionExpression function_expression)
{
    /* Canonize before uniqueness test. */
    std::sort(effects.begin(), effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<EffectStripsImpl> {}).get_or_create(std::move(effects), std::move(function_expression));
}

EffectConditional PDDLRepositories::get_or_create_conditional_effect(VariableList parameters,
                                                                     LiteralList<Static> static_conditions,
                                                                     LiteralList<Fluent> fluent_conditions,
                                                                     LiteralList<Derived> derived_conditions,
                                                                     LiteralList<Fluent> effects,
                                                                     FunctionExpression function_expression)
{
    /* Canonize before uniqueness test. */
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(effects.begin(), effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<EffectConditionalImpl> {})
        .get_or_create(std::move(parameters),
                       std::move(static_conditions),
                       std::move(fluent_conditions),
                       std::move(derived_conditions),
                       std::move(effects),
                       std::move(function_expression));
}

ExistentiallyQuantifiedConjunctiveCondition
PDDLRepositories::get_or_create_existentially_quantified_conjunctive_condition(VariableList parameters,
                                                                               LiteralList<Static> static_conditions,
                                                                               LiteralList<Fluent> fluent_conditions,
                                                                               LiteralList<Derived> derived_conditions)
{
    /* Canonize before uniqueness test */
    std::sort(static_conditions.begin(), static_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_conditions.begin(), fluent_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_conditions.begin(), derived_conditions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    auto nullary_static_conditions = ground_nullary_literals(static_conditions, *this);
    auto nullary_fluent_conditions = ground_nullary_literals(fluent_conditions, *this);
    auto nullary_derived_conditions = ground_nullary_literals(derived_conditions, *this);

    return boost::hana::at_key(m_repositories, boost::hana::type<ExistentiallyQuantifiedConjunctiveConditionImpl> {})
        .get_or_create(std::move(parameters),
                       std::move(static_conditions),
                       std::move(fluent_conditions),
                       std::move(derived_conditions),
                       std::move(nullary_static_conditions),
                       std::move(nullary_fluent_conditions),
                       std::move(nullary_derived_conditions));
}

Action PDDLRepositories::get_or_create_action(std::string name,
                                              size_t original_arity,
                                              ExistentiallyQuantifiedConjunctiveCondition precondition,
                                              EffectStrips strips_effect,
                                              EffectConditionalList conditional_effects)
{
    /* Canonize before uniqueness test */
    std::sort(conditional_effects.begin(), conditional_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {})
        .get_or_create(std::move(name), original_arity, std::move(precondition), std::move(strips_effect), std::move(conditional_effects));
}

Axiom PDDLRepositories::get_or_create_axiom(ExistentiallyQuantifiedConjunctiveCondition precondition, Literal<Derived> literal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).get_or_create(std::move(precondition), std::move(literal));
}

OptimizationMetric PDDLRepositories::get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<OptimizationMetricImpl> {}).get_or_create(std::move(metric), std::move(function_expression));
}

NumericFluent PDDLRepositories::get_or_create_numeric_fluent(GroundFunction function, double number)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericFluentImpl> {}).get_or_create(std::move(function), std::move(number));
}

Domain PDDLRepositories::get_or_create_domain(std::optional<fs::path> filepath,
                                              std::string name,
                                              Requirements requirements,
                                              ObjectList constants,
                                              PredicateList<Static> static_predicates,
                                              PredicateList<Fluent> fluent_predicates,
                                              PredicateList<Derived> derived_predicates,
                                              FunctionSkeletonList functions,
                                              ActionList actions,
                                              AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(constants.begin(), constants.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_predicates.begin(), static_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_predicates.begin(), fluent_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(functions.begin(), functions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
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
                       std::move(functions),
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
                                                NumericFluentList numeric_fluents,
                                                GroundLiteralList<Static> static_goal_condition,
                                                GroundLiteralList<Fluent> fluent_goal_condition,
                                                GroundLiteralList<Derived> derived_goal_condition,
                                                std::optional<OptimizationMetric> optimization_metric,
                                                AxiomList axioms)
{
    /* Canonize before uniqueness test. */
    std::sort(objects.begin(), objects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_predicates.begin(), derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_initial_literals.begin(), static_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_initial_literals.begin(), fluent_initial_literals.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(numeric_fluents.begin(), numeric_fluents.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(static_goal_condition.begin(), static_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(fluent_goal_condition.begin(), fluent_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(derived_goal_condition.begin(), derived_goal_condition.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
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
                       std::move(numeric_fluents),
                       std::move(static_goal_condition),
                       std::move(fluent_goal_condition),
                       std::move(derived_goal_condition),
                       std::move(optimization_metric),
                       std::move(axioms));
}

// Factory
const PDDLTypeToRepository& PDDLRepositories::get_pddl_type_to_factory() const { return m_repositories; }

// GroundAtom
template<PredicateTag P>
GroundAtom<P> PDDLRepositories::get_ground_atom(size_t atom_index) const
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).at(atom_index);
}

template GroundAtom<Static> PDDLRepositories::get_ground_atom<Static>(size_t atom_index) const;
template GroundAtom<Fluent> PDDLRepositories::get_ground_atom<Fluent>(size_t atom_index) const;
template GroundAtom<Derived> PDDLRepositories::get_ground_atom<Derived>(size_t atom_index) const;

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
