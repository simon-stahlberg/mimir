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

#include "mimir/formalism/parser.hpp"

#include "mimir/formalism/domain_builder.hpp"
#include "mimir/formalism/problem_builder.hpp"
#include "mimir/formalism/translator/encode_parameter_index_in_variables.hpp"
#include "to_mimir_structures.hpp"

#include <memory>

namespace mimir::formalism
{

Parser::Parser(const fs::path& domain_filepath, const loki::ParserOptions& options) :
    m_loki_parser(domain_filepath, options),
    m_loki_domain_translation_result(loki::translate(m_loki_parser.get_domain())),
    m_domain()
{
    translate_domain();
}

Parser::Parser(const std::string& domain_content, const fs::path& domain_filepath, const loki::ParserOptions& options) :
    m_loki_parser(domain_content, domain_filepath, options),
    m_loki_domain_translation_result(loki::translate(m_loki_parser.get_domain())),
    m_domain()
{
    translate_domain();
}

Problem Parser::parse_problem(const fs::path& problem_filepath, const loki::ParserOptions& options)
{
    auto loki_problem = m_loki_parser.parse_problem(problem_filepath, options);
    return translate_problem(loki_problem);
}

Problem Parser::parse_problem(const std::string& problem_content, const fs::path& problem_filepath, const loki::ParserOptions& options)
{
    auto loki_problem = m_loki_parser.parse_problem(problem_content, problem_filepath, options);
    return translate_problem(loki_problem);
}

const Domain& Parser::get_domain() const { return m_domain; }

void Parser::translate_domain()
{
    auto loki_translated_domain = m_loki_domain_translation_result.get_translated_domain();

    auto to_mimir_structures_translator = ToMimirStructures();
    auto builder = DomainBuilder();
    m_domain = to_mimir_structures_translator.translate(loki_translated_domain, builder);

    auto encode_parameter_index_in_variables_translator = EncodeParameterIndexInVariables();
    builder = DomainBuilder();
    m_domain = encode_parameter_index_in_variables_translator.translate_level_0(m_domain, builder);
}

Problem Parser::translate_problem(const loki::Problem& loki_problem)
{
    auto loki_translated_problem = loki::translate(loki_problem, m_loki_domain_translation_result);

    auto to_mimir_structures_translator = ToMimirStructures();
    auto builder = ProblemBuilder(m_domain);
    auto problem = to_mimir_structures_translator.translate(loki_translated_problem, builder);

    auto encode_parameter_index_in_variables_translator = EncodeParameterIndexInVariables();
    builder = ProblemBuilder(m_domain);
    problem = encode_parameter_index_in_variables_translator.translate_level_0(problem, builder);

    return problem;
}

}
