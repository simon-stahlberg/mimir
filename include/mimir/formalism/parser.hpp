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
#include "mimir/formalism/factories.hpp"

#include <loki/utils/filesystem.hpp>

namespace mimir
{

class PDDLParser
{
private:
    PDDLFactories m_factories;

    // Parsed result
    Domain m_domain;
    Problem m_problem;

public:
    PDDLParser(const fs::path& domain_file_path, const fs::path& problem_file_path);

    /// @brief Get the factories to create additional PDDL objects.
    PDDLFactories& get_factories();

    /// @brief Get the parsed domain.
    const Domain& get_domain() const;

    /// @brief Get the parsed problem.
    const Problem& get_problem() const;
};

}

#endif