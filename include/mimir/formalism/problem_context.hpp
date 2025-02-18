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

#ifndef MIMIR_FORMALISM_PROBLEM_CONTEXT_HPP_
#define MIMIR_FORMALISM_PROBLEM_CONTEXT_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ProblemContext
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_repositories;

public:
    ProblemContext(const fs::path& domain_filepath, const fs::path& problem_filepath);
    ProblemContext(Problem problem, std::shared_ptr<PDDLRepositories> repositories);

    static std::vector<ProblemContext> create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths);
    static std::vector<ProblemContext> create(const fs::path& domain_filepath, const fs::path& problems_directory);

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_repositories() const;
};

using ProblemContextList = std::vector<ProblemContext>;
}

#endif
