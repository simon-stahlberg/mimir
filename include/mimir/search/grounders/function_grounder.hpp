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

#ifndef MIMIR_SEARCH_GROUNDERS_FUNCTION_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDERS_FUNCTION_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounders/grounding_table.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
class FunctionGrounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    GroundingTableList<GroundFunction> m_grounding_tables;

public:
    /// @brief Simplest construction
    FunctionGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    FunctionGrounder(const FunctionGrounder& other) = delete;
    FunctionGrounder& operator=(const FunctionGrounder& other) = delete;
    // Moveable
    FunctionGrounder(FunctionGrounder&& other) = default;
    FunctionGrounder& operator=(FunctionGrounder&& other) = default;

    GroundFunction ground_function(Function function, const ObjectList& binding);

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
};

}  // namespace mimir

#endif
