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

#ifndef MIMIR_FORMALISM_PARSER_HPP_
#define MIMIR_FORMALISM_PARSER_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>
#include <memory>

namespace mimir::formalism
{

class Parser
{
public:
    Parser(const fs::path& domain_filepath, const loki::Options& options = loki::Options());

    Problem parse_problem(const fs::path& problem_filepath, const loki::Options& options = loki::Options());

    const Domain& get_domain() const;

private:
    loki::Parser m_loki_parser;
    loki::DomainTranslationResult m_loki_domain_translation_result;

    Domain m_domain;  ///< The parsed domain.
};

}

#endif
