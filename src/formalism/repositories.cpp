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
#include "mimir/formalism/formatter.hpp"

namespace mimir::formalism
{

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

template<IsStaticOrFluentOrDerivedTag P>
static GroundLiteralList<P> ground_nullary_literals(const LiteralList<P>& literals, Repositories& repositories)
{
    auto ground_literals = GroundLiteralList<P> {};
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_arity() != 0)
            continue;

        ground_literals.push_back(repositories.get_or_create_ground_literal(literal->get_polarity(),
                                                                            repositories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), {})));
    }

    return ground_literals;
}

static GroundNumericConstraintList ground_nullary_constraints(const NumericConstraintList& constraints, Repositories& repositories)
{
    auto ground_constraints = GroundNumericConstraintList {};
    for (const auto& constraint : constraints)
    {
        if (constraint->get_arity() != 0)
            continue;

        ground_constraints.push_back(repositories.ground(constraint, {}));
    }

    return ground_constraints;
}

template<IsStaticOrFluentOrDerivedTag P>
GroundLiteral<P> Repositories::ground(Literal<P> literal, const ObjectList& binding)
{
    // We have to fetch the literal-relevant part of the binding first.
    static thread_local auto s_grounded_terms = ObjectList {};
    ground_terms(literal->get_atom()->get_terms(), binding, s_grounded_terms);

    return get_or_create_ground_literal(literal->get_polarity(), get_or_create_ground_atom(literal->get_atom()->get_predicate(), s_grounded_terms));
}

template GroundLiteral<StaticTag> Repositories::ground(Literal<StaticTag> literal, const ObjectList& binding);
template GroundLiteral<FluentTag> Repositories::ground(Literal<FluentTag> literal, const ObjectList& binding);
template GroundLiteral<DerivedTag> Repositories::ground(Literal<DerivedTag> literal, const ObjectList& binding);

GroundFunctionExpression Repositories::ground(FunctionExpression fexpr, const ObjectList& binding)
{
    return std::visit(
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
                const auto ground_lhs = ground(arg->get_left_function_expression(), binding);
                const auto ground_rhs = ground(arg->get_right_function_expression(), binding);

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
                    const auto ground_child_fexpr = ground(child_fexpr, binding);
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
                const auto ground_fexpr = ground(arg->get_function_expression(), binding);

                return std::holds_alternative<GroundFunctionExpressionNumber>(ground_fexpr->get_variant()) ?
                           get_or_create_ground_function_expression(get_or_create_ground_function_expression_number(
                               -std::get<GroundFunctionExpressionNumber>(ground_fexpr->get_variant())->get_number())) :
                           ground_fexpr;
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<StaticTag>>)
            {
                return get_or_create_ground_function_expression(
                    get_or_create_ground_function_expression_function<StaticTag>(ground(arg->get_function(), binding)));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<FluentTag>>)
            {
                return get_or_create_ground_function_expression(
                    get_or_create_ground_function_expression_function<FluentTag>(ground(arg->get_function(), binding)));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "NumericConstraintGrounder::ground(fexpr, binding): Missing implementation for GroundFunctionExpression type.");
            }
        },
        fexpr->get_variant());
}

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> Repositories::ground(Function<F> function, const ObjectList& binding)
{
    // We have to fetch the function-relevant part of the binding first.
    static thread_local auto s_grounded_terms = ObjectList {};
    ground_terms(function->get_terms(), binding, s_grounded_terms);

    return get_or_create_ground_function(function->get_function_skeleton(), s_grounded_terms);
}

template GroundFunction<StaticTag> Repositories::ground(Function<StaticTag> function, const ObjectList& binding);
template GroundFunction<FluentTag> Repositories::ground(Function<FluentTag> function, const ObjectList& binding);
template GroundFunction<AuxiliaryTag> Repositories::ground(Function<AuxiliaryTag> function, const ObjectList& binding);

GroundNumericConstraint Repositories::ground(NumericConstraint numeric_constraint, const ObjectList& binding)
{
    return get_or_create_ground_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                   ground(numeric_constraint->get_left_function_expression(), binding),
                                                   ground(numeric_constraint->get_right_function_expression(), binding));
}

// NumericEffect

template<IsFluentOrAuxiliaryTag F>
GroundNumericEffect<F> Repositories::ground(NumericEffect<F> numeric_effect, const ObjectList& binding)
{
    return get_or_create_ground_numeric_effect(numeric_effect->get_assign_operator(),
                                               ground(numeric_effect->get_function(), binding),
                                               ground(numeric_effect->get_function_expression(), binding));
}

template GroundNumericEffect<FluentTag> Repositories::ground(NumericEffect<FluentTag> numeric_effect, const ObjectList& binding);
template GroundNumericEffect<AuxiliaryTag> Repositories::ground(NumericEffect<AuxiliaryTag> numeric_effect, const ObjectList& binding);

Repositories::Repositories(const Repositories* parent) :
    m_repositories(boost::hana::unpack(boost::hana::transform(boost::hana::keys(HanaRepositories {}),
                                                              [=](auto type_c)
                                                              {
                                                                  using T = typename decltype(type_c)::type;
                                                                  using Repo = loki::IndexedHashSet<T>;

                                                                  const Repo* parent_repo = nullptr;
                                                                  if (parent)
                                                                  {
                                                                      parent_repo = &boost::hana::at_key(parent->m_repositories, type_c);
                                                                  }

                                                                  return boost::hana::make_pair(type_c, Repo { parent_repo });
                                                              }),
                                       boost::hana::make_map))
{
}

HanaRepositories& Repositories::get_hana_repositories() { return m_repositories; }

const HanaRepositories& Repositories::get_hana_repositories() const { return m_repositories; }

Requirements Repositories::get_or_create_requirements(loki::RequirementEnumSet requirement_set)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RequirementsImpl> {}).get_or_create(std::move(requirement_set));
}

Type Repositories::get_or_create_type(std::string name, TypeList bases)
{
    std::sort(bases.begin(), bases.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_name() < rhs->get_name(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<TypeImpl> {}).get_or_create(std::move(name), std::move(bases));
}

Variable Repositories::get_or_create_variable(std::string name, size_t parameter_index)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<VariableImpl> {}).get_or_create(std::move(name), std::move(parameter_index));
}

Parameter Repositories::get_or_create_parameter(Variable variable, TypeList types)
{
    std::sort(types.begin(), types.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_name() < rhs->get_name(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ParameterImpl> {}).get_or_create(std::move(variable), std::move(types));
}

Term Repositories::get_or_create_term(Variable variable)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(variable));
}

Term Repositories::get_or_create_term(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<TermImpl> {}).get_or_create(std::move(object));
}

Object Repositories::get_or_create_object(std::string name, TypeList types)
{
    std::sort(types.begin(), types.end(), [](const auto& lhs, const auto& rhs) { return lhs->get_name() < rhs->get_name(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ObjectImpl> {}).get_or_create(std::move(name), std::move(types));
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
GroundAtom<P> Repositories::get_or_create_ground_atom(Predicate<P> predicate, const ObjectList& binding)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}).get_or_create(predicate, binding);
}

template GroundAtom<StaticTag> Repositories::get_or_create_ground_atom(Predicate<StaticTag> predicate, const ObjectList& binding);
template GroundAtom<FluentTag> Repositories::get_or_create_ground_atom(Predicate<FluentTag> predicate, const ObjectList& binding);
template GroundAtom<DerivedTag> Repositories::get_or_create_ground_atom(Predicate<DerivedTag> predicate, const ObjectList& binding);

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
Predicate<P> Repositories::get_or_create_predicate(std::string name, ParameterList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<PredicateImpl<P>> {}).get_or_create(name, std::move(parameters));
}

template Predicate<StaticTag> Repositories::get_or_create_predicate(std::string name, ParameterList parameters);
template Predicate<FluentTag> Repositories::get_or_create_predicate(std::string name, ParameterList parameters);
template Predicate<DerivedTag> Repositories::get_or_create_predicate(std::string name, ParameterList parameters);

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
Function<F> Repositories::get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionImpl<F>> {}).get_or_create(std::move(function_skeleton), std::move(terms));
}

template Function<StaticTag> Repositories::get_or_create_function(FunctionSkeleton<StaticTag> function_skeleton, TermList terms);
template Function<FluentTag> Repositories::get_or_create_function(FunctionSkeleton<FluentTag> function_skeleton, TermList terms);
template Function<AuxiliaryTag> Repositories::get_or_create_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, TermList terms);

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> Repositories::get_or_create_ground_function(FunctionSkeleton<F> function_skeleton, const ObjectList& binding)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundFunctionImpl<F>> {}).get_or_create(function_skeleton, binding);
}

template GroundFunction<StaticTag> Repositories::get_or_create_ground_function(FunctionSkeleton<StaticTag> function_skeleton, const ObjectList& binding);
template GroundFunction<FluentTag> Repositories::get_or_create_ground_function(FunctionSkeleton<FluentTag> function_skeleton, const ObjectList& binding);
template GroundFunction<AuxiliaryTag> Repositories::get_or_create_ground_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, const ObjectList& binding);

template<IsStaticOrFluentOrAuxiliaryTag F>
FunctionSkeleton<F> Repositories::get_or_create_function_skeleton(std::string name, ParameterList parameters)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<FunctionSkeletonImpl<F>> {}).get_or_create(std::move(name), std::move(parameters));
}

template FunctionSkeleton<StaticTag> Repositories::get_or_create_function_skeleton(std::string name, ParameterList parameters);
template FunctionSkeleton<FluentTag> Repositories::get_or_create_function_skeleton(std::string name, ParameterList parameters);
template FunctionSkeleton<AuxiliaryTag> Repositories::get_or_create_function_skeleton(std::string name, ParameterList parameters);

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

ConjunctiveEffect Repositories::get_or_create_conjunctive_effect(ParameterList parameters,
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

GroundConjunctiveEffect
Repositories::get_or_create_ground_conjunctive_effect(HanaContainer<const FlatIndexList*, PositiveTag, NegativeTag> propositional_effects,
                                                      GroundNumericEffectList<FluentTag> fluent_numeric_effects,
                                                      std::optional<const GroundNumericEffect<AuxiliaryTag>> auxiliary_numeric_effect)
{
    /* Canonize before uniqueness test. */
    std::sort(fluent_numeric_effects.begin(), fluent_numeric_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<GroundConjunctiveEffectImpl> {})
        .get_or_create(std::move(propositional_effects), std::move(fluent_numeric_effects), std::move(auxiliary_numeric_effect));
}

ConditionalEffect Repositories::get_or_create_conditional_effect(ConjunctiveCondition conjunctive_condition, ConjunctiveEffect conjunctive_effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConditionalEffectImpl> {}).get_or_create(conjunctive_condition, conjunctive_effect);
}

GroundConditionalEffect Repositories::get_or_create_ground_conditional_effect(GroundConjunctiveCondition conjunctive_condition,
                                                                              GroundConjunctiveEffect conjunctive_effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundConditionalEffectImpl> {}).get_or_create(conjunctive_condition, conjunctive_effect);
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

ConjunctiveCondition Repositories::get_or_create_conjunctive_condition(ParameterList parameters,
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

    auto nullary_ground_constraints = ground_nullary_constraints(numeric_constraints, *this);

    std::sort(numeric_constraints.begin(), numeric_constraints.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(nullary_ground_constraints.begin(),
              nullary_ground_constraints.end(),
              [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ConjunctiveConditionImpl> {})
        .get_or_create(std::move(parameters),
                       std::move(literals),
                       std::move(nullary_ground_literals),
                       std::move(numeric_constraints),
                       std::move(nullary_ground_constraints));
}

GroundConjunctiveCondition Repositories::get_or_create_ground_conjunctive_condition(
    HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> preconditions,
    GroundNumericConstraintList numeric_constraints)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundConjunctiveConditionImpl> {})
        .get_or_create(preconditions, std::move(numeric_constraints));
}

Action Repositories::get_or_create_action(std::string name,
                                          size_t original_arity,
                                          ConjunctiveCondition conjunctive_condition,
                                          ConditionalEffectList conditional_effects)
{
    /* Canonize before uniqueness test */
    std::sort(conditional_effects.begin(), conditional_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<ActionImpl> {})
        .get_or_create(std::move(name), original_arity, conjunctive_condition, std::move(conditional_effects));
}

GroundAction Repositories::get_or_create_ground_action(Action action,
                                                       const ObjectList& binding,
                                                       GroundConjunctiveCondition condition,
                                                       GroundConditionalEffectList conditional_effects)
{
    /* Canonize before uniqueness test */
    std::sort(conditional_effects.begin(), conditional_effects.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    return boost::hana::at_key(m_repositories, boost::hana::type<GroundActionImpl> {}).get_or_create(action, binding, condition, conditional_effects);
}

Axiom Repositories::get_or_create_axiom(ConjunctiveCondition conjunctive_condition, Literal<DerivedTag> literal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<AxiomImpl> {}).get_or_create(std::move(conjunctive_condition), std::move(literal));
}

GroundAxiom
Repositories::get_or_create_ground_axiom(Axiom axiom, const ObjectList& binding, GroundConjunctiveCondition condition, GroundLiteral<DerivedTag> effect)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<GroundAxiomImpl> {}).get_or_create(axiom, binding, condition, effect);
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

    for (size_t i = 0; i < num_ground_functions; ++i)
        out_ground_functions.push_back(ground_functions[i]);
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
        out_ground_function_values.emplace_back(ground_functions.at(i), values[i]);
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
