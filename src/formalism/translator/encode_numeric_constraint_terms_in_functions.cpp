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

#include "mimir/formalism/translator/encode_numeric_constraint_terms_in_functions.hpp"

#include "mimir/common/collections.hpp"

namespace mimir::formalism
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
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<StaticTag>> || std::is_same_v<T, FunctionExpressionFunction<FluentTag>>
                               || std::is_same_v<T, FunctionExpressionFunction<AuxiliaryTag>>)
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

template<IsStaticOrFluentOrAuxiliaryTag F>
Function<F> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<F> function, Repositories& repositories)
{
    auto transformed_function_skeleton = this->translate_level_0(function->get_function_skeleton(), repositories);
    auto transformed_terms = this->translate_level_0(function->get_terms(), repositories);

    auto transformed_term_to_index = std::unordered_map<Term, Index> {};
    for (size_t i = 0; i < transformed_terms.size(); ++i)
    {
        transformed_term_to_index.emplace(transformed_terms[i], i);
    }

    // Note: we initialize with -1, because not all mappings from parent to child might be defined.
    auto parent_terms_to_terms_mapping = IndexList(m_numeric_constraint_terms.size(), -1);
    for (size_t j = 0; j < m_numeric_constraint_terms.size(); ++j)
    {
        const auto& parent_term = m_numeric_constraint_terms[j];

        if (transformed_term_to_index.contains(parent_term))
        {
            // point j-th parent to i-th child
            parent_terms_to_terms_mapping[j] = transformed_term_to_index.at(parent_term);
        }
    }

    return repositories.get_or_create_function(transformed_function_skeleton, transformed_terms, parent_terms_to_terms_mapping);
}

template Function<StaticTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<StaticTag> function, Repositories& repositories);
template Function<FluentTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<FluentTag> function, Repositories& repositories);
template Function<AuxiliaryTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<AuxiliaryTag> function, Repositories& repositories);

NumericConstraint EncodeNumericConstraintTermsInFunctions::translate_level_2(NumericConstraint numeric_constraint, Repositories& repositories)
{
    assert(numeric_constraint->get_terms().empty());
    auto terms = TermList {};
    collect_terms(numeric_constraint->get_left_function_expression(), terms);
    collect_terms(numeric_constraint->get_right_function_expression(), terms);
    terms = uniquify_elements(terms);
    terms = this->translate_level_0(terms, repositories);
    std::sort(terms.begin(), terms.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    // Set the member variable to be used in nested tranformations.
    m_numeric_constraint_terms = terms;

    return repositories.get_or_create_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                         this->translate_level_0(numeric_constraint->get_left_function_expression(), repositories),
                                                         this->translate_level_0(numeric_constraint->get_right_function_expression(), repositories),
                                                         terms);
}

}
