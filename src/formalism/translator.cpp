/*
 * Copyright (C) 2023 Dominik Drexler
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

#include "mimir/formalism/translator.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/domain_builder.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/problem_builder.hpp"
#include "mimir/formalism/translator/encode_numeric_constraint_terms_in_functions.hpp"
#include "mimir/formalism/translator/encode_parameter_index_in_variables.hpp"

namespace mimir::formalism
{

Translator::Translator(const Domain& domain) : m_original_domain(domain)
{
    auto encode_parameter_index_in_variables_translator = EncodeParameterIndexInVariables();
    auto builder = DomainBuilder();
    m_translated_domain = encode_parameter_index_in_variables_translator.translate_level_0(domain, builder);

    auto encode_numeric_constraint_terms_in_function = EncodeNumericConstraintTermsInFunctions();
    builder = DomainBuilder();
    m_translated_domain = encode_numeric_constraint_terms_in_function.translate_level_0(m_translated_domain, builder);
}

Problem Translator::translate(const Problem& problem) const
{
    if (get_original_domain() != problem->get_domain())
    {
        throw std::runtime_error("Translator::translate: domain in problem must match original domain in DomainTranslationResult.");
    }

    // std::cout << "EncodeNumericConstraintTermsInFunctions" << std::endl;

    auto encode_parameter_index_in_variables_translator = EncodeNumericConstraintTermsInFunctions();
    auto builder = ProblemBuilder(get_translated_domain());
    auto translated_problem = encode_parameter_index_in_variables_translator.translate_level_0(problem, builder);

    // std::cout << "EncodeParameterIndexInVariables" << std::endl;

    auto encode_numeric_constraint_terms_in_function = EncodeParameterIndexInVariables();
    builder = ProblemBuilder(get_translated_domain());
    translated_problem = encode_numeric_constraint_terms_in_function.translate_level_0(problem, builder);

    return translated_problem;
}

const Domain& Translator::get_original_domain() const { return m_original_domain; }

const Domain& Translator::get_translated_domain() const { return m_translated_domain; }

}
