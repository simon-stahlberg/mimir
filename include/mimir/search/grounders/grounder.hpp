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

#ifndef MIMIR_SEARCH_GROUNDERS_GROUNDER_HPP_
#define MIMIR_SEARCH_GROUNDERS_GROUNDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/grounders/action_grounder.hpp"
#include "mimir/search/grounders/axiom_grounder.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
class Grounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    LiteralGrounder m_literal_grounder;
    ActionGrounder m_action_grounder;
    AxiomGrounder m_axiom_ground;

public:
    /// @brief Simplest construction
    Grounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    Grounder(const Grounder& other) = delete;
    Grounder& operator=(const Grounder& other) = delete;
    // Moveable
    Grounder(Grounder&& other) = default;
    Grounder& operator=(Grounder&& other) = default;

    LiteralGrounder& get_literal_grounder();
    ActionGrounder& get_action_grounder();
    AxiomGrounder& get_axiom_grounder();

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
};

}  // namespace mimir

#endif
