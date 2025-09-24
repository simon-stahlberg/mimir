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

template<IsStaticOrFluentOrAuxiliaryTag F>
Function<F> EncodeNumericConstraintTermsInFunctions::translate_level_2_impl(Function<F> function, Repositories& repositories)
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

template Function<StaticTag> EncodeNumericConstraintTermsInFunctions::translate_level_2_impl(Function<StaticTag> function, Repositories& repositories);
template Function<FluentTag> EncodeNumericConstraintTermsInFunctions::translate_level_2_impl(Function<FluentTag> function, Repositories& repositories);
template Function<AuxiliaryTag> EncodeNumericConstraintTermsInFunctions::translate_level_2_impl(Function<AuxiliaryTag> function, Repositories& repositories);

Function<StaticTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<StaticTag> function, Repositories& repositories)
{
    return translate_level_2_impl(function, repositories);
}

Function<FluentTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<FluentTag> function, Repositories& repositories)
{
    return translate_level_2_impl(function, repositories);
}

Function<AuxiliaryTag> EncodeNumericConstraintTermsInFunctions::translate_level_2(Function<AuxiliaryTag> function, Repositories& repositories)
{
    return translate_level_2_impl(function, repositories);
}

NumericConstraint EncodeNumericConstraintTermsInFunctions::translate_level_2(NumericConstraint numeric_constraint, Repositories& repositories)
{
    // Set the member variable to be used in nested tranformations.
    m_numeric_constraint_terms = this->translate_level_0(numeric_constraint->get_terms(), repositories);

    return repositories.get_or_create_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                         this->translate_level_0(numeric_constraint->get_left_function_expression(), repositories),
                                                         this->translate_level_0(numeric_constraint->get_right_function_expression(), repositories),
                                                         m_numeric_constraint_terms);
}

}
