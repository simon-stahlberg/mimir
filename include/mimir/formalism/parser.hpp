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

#ifndef MIMIR_FORMALISM_PARSER_HPP_
#define MIMIR_FORMALISM_PARSER_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/pddl.hpp"

#include <loki/loki.hpp>

namespace mimir
{

class PDDLParser
{
private:
    // Meta data
    fs::path m_domain_filepath;
    fs::path m_problem_filepath;

    // Parsers that contain the original domain and problem
    loki::DomainParser m_loki_domain_parser;
    loki::ProblemParser m_loki_problem_parser;

    // The translated representation
    std::shared_ptr<PDDLFactories> m_factories;
    Domain m_domain;
    Problem m_problem;

public:
    PDDLParser(const fs::path& domain_filepath, const fs::path& problem_filepath);

    /// @brief Get the domain filepath.
    const fs::path& get_domain_filepath() const;

    /// @brief Get the problem filepath.
    const fs::path& get_problem_filepath() const;

    /// @brief Get the original domain.
    const loki::Domain get_original_domain() const;

    /// @brief Get the original problem.
    const loki::Problem get_original_problem() const;

    /// @brief Get the factories to create additional PDDL objects.
    const std::shared_ptr<PDDLFactories>& get_factories();

    /// @brief Get the translated domain.
    const Domain& get_domain() const;

    /// @brief Get the translated problem.
    const Problem& get_problem() const;
};

}

#endif