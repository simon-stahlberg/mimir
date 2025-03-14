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
#include "mimir/formalism/translator.hpp"
#include "to_mimir_structures.hpp"

#include <memory>

namespace mimir::formalism
{

Parser::Parser(const fs::path& domain_filepath, const loki::Options& options) :
    m_loki_parser(domain_filepath, options),
    m_loki_domain_translation_result(loki::translate(m_loki_parser.get_domain()))
{
    auto loki_translated_domain = m_loki_domain_translation_result.get_translated_domain();

    auto to_mimir_structures_translator = ToMimirStructures();
    auto builder = DomainBuilder();
    m_domain = to_mimir_structures_translator.translate(loki_translated_domain, builder);
}

Problem Parser::parse_problem(const fs::path& problem_filepath, const loki::Options& options)
{
    auto loki_problem = m_loki_parser.parse_problem(problem_filepath, options);
    auto loki_translated_problem = loki::translate(loki_problem, m_loki_domain_translation_result);

    auto to_mimir_structures_translator = ToMimirStructures();
    auto builder = ProblemBuilder(m_domain);

    return to_mimir_structures_translator.translate(loki_translated_problem, builder);
}

const Domain& Parser::get_domain() const { return m_domain; }

}
