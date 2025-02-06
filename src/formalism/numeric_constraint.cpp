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

#include "mimir/formalism/numeric_constraint.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/term.hpp"

namespace mimir
{

static void collect_terms(FunctionExpression fexpr, TermList& ref_terms)
{
    std::visit(
        [&ref_terms](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>) {}
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                collect_terms(arg->get_left_function_expression(), ref_terms);
                collect_terms(arg->get_right_function_expression(), ref_terms);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                for (const auto& part : arg->get_function_expressions())
                {
                    collect_terms(part, ref_terms);
                }
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                collect_terms(arg->get_function_expression(), ref_terms);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Static>> || std::is_same_v<T, FunctionExpressionFunction<Fluent>>
                               || std::is_same_v<T, FunctionExpressionFunction<Auxiliary>>)
            {
                ref_terms.insert(ref_terms.end(), arg->get_function()->get_terms().begin(), arg->get_function()->get_terms().end());
            }
            else
            {
                static_assert(dependent_false<T>::value, "collect_terms_helper(fexpr, ref_terms): Missing implementation for FunctionExpression type.");
            }
        },
        fexpr->get_variant());
}

template<StaticOrFluentTag F>
static void compute_remapping(FunctionExpressionFunction<F> fexpr, const TermList& terms, absl::flat_hash_map<Function<F>, IndexList>& remapping)
{
    if (remapping.contains(fexpr->get_function()))
        return;

    auto term_to_position = absl::flat_hash_map<Term, Index> {};
    for (size_t i = 0; i < fexpr->get_function()->get_terms().size(); ++i)
    {
        term_to_position.emplace(fexpr->get_function()->get_terms()[i], i);
    }

    auto mapping = IndexList {};
    for (size_t i = 0; i < terms.size(); ++i)
    {
        mapping.push_back(term_to_position.contains(terms[i]) ? term_to_position.at(terms[i]) : std::numeric_limits<Index>::max());
    }

    remapping[fexpr->get_function()] = std::move(mapping);
}

static void computing_remapping(FunctionExpression fexpr,
                                const TermList& terms,
                                absl::flat_hash_map<Function<Static>, IndexList>& static_remapping,
                                absl::flat_hash_map<Function<Fluent>, IndexList>& fluent_remapping)
{
    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>) {}
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                computing_remapping(arg->get_left_function_expression(), terms, static_remapping, fluent_remapping);
                computing_remapping(arg->get_right_function_expression(), terms, static_remapping, fluent_remapping);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                for (const auto& part : arg->get_function_expressions())
                {
                    computing_remapping(part, terms, static_remapping, fluent_remapping);
                }
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                computing_remapping(arg->get_function_expression(), terms, static_remapping, fluent_remapping);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Static>>)
            {
                compute_remapping(arg, terms, static_remapping);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Fluent>>)
            {
                compute_remapping(arg, terms, fluent_remapping);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<Auxiliary>>)
            {
                throw std::runtime_error("computing_remapping(fexpr, term_to_position, fluent_remapping): Unexpected FunctionExpression type.");
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "computing_remapping(fexpr, term_to_position, fluent_remapping): Missing implementation for FunctionExpression type.");
            }
        },
        fexpr->get_variant());
}

NumericConstraintImpl::NumericConstraintImpl(Index index,
                                             loki::BinaryComparatorEnum binary_comparator,
                                             FunctionExpression left_function_expression,
                                             FunctionExpression right_function_expression) :
    m_index(index),
    m_binary_comparator(binary_comparator),
    m_left_function_expression(left_function_expression),
    m_right_function_expression(right_function_expression),
    m_terms(),
    m_static_remapping(),
    m_fluent_remapping()
{
    collect_terms(m_left_function_expression, m_terms);
    collect_terms(m_right_function_expression, m_terms);
    m_terms = uniquify_elements(m_terms);

    computing_remapping(m_left_function_expression, m_terms, m_static_remapping, m_fluent_remapping);
    computing_remapping(m_right_function_expression, m_terms, m_static_remapping, m_fluent_remapping);
}

Index NumericConstraintImpl::get_index() const { return m_index; }

loki::BinaryComparatorEnum NumericConstraintImpl::get_binary_comparator() const { return m_binary_comparator; }

const FunctionExpression& NumericConstraintImpl::get_left_function_expression() const { return m_left_function_expression; }

const FunctionExpression& NumericConstraintImpl::get_right_function_expression() const { return m_right_function_expression; }

const TermList& NumericConstraintImpl::get_terms() const { return m_terms; }

template<StaticOrFluentTag F>
const absl::flat_hash_map<Function<F>, IndexList>& NumericConstraintImpl::get_remapping() const
{
    if constexpr (std::is_same_v<F, Static>)
    {
        return m_static_remapping;
    }
    else if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_remapping;
    }
    else
    {
        static_assert(dependent_false<F>::value, "Missing implementation for FunctionTag.");
    }
}

template const absl::flat_hash_map<Function<Static>, IndexList>& NumericConstraintImpl::get_remapping() const;
template const absl::flat_hash_map<Function<Fluent>, IndexList>& NumericConstraintImpl::get_remapping() const;

std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, NumericConstraint element)
{
    out << *element;
    return out;
}
}
