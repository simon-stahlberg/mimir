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

#ifndef MIMIR_FORMALISM_GENERALIZED_PROBLEM_HPP_
#define MIMIR_FORMALISM_GENERALIZED_PROBLEM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class GeneralizedProblemImpl
{
private:
    Domain m_domain;
    ProblemList m_problems;

    GeneralizedProblemImpl(Domain domain, ProblemList problems);

public:
    static GeneralizedProblem
    create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const loki::Options& options = loki::Options());
    static GeneralizedProblem create(const fs::path& domain_filepath, const fs::path& problems_directory, const loki::Options& options = loki::Options());
    static GeneralizedProblem create(Domain domain, ProblemList problems);

    const Domain& get_domain() const;
    const ProblemList& get_problems() const;
};

}

#endif