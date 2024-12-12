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
#include "mimir/search/grounders/function_grounder.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"

namespace mimir
{
/// @brief `Grounder` encapsulates instantiation and access to all specific grounders.
class Grounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    std::shared_ptr<LiteralGrounder> m_literal_grounder;
    std::shared_ptr<FunctionGrounder> m_function_grounder;
    std::shared_ptr<ActionGrounder> m_action_grounder;
    std::shared_ptr<AxiomGrounder> m_axiom_grounder;

public:
    /// @brief Simplest construction
    Grounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    Grounder(const Grounder& other) = delete;
    Grounder& operator=(const Grounder& other) = delete;
    // Moveable
    Grounder(Grounder&& other) = default;
    Grounder& operator=(Grounder&& other) = default;

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;

    const std::shared_ptr<LiteralGrounder>& get_literal_grounder() const;
    const std::shared_ptr<FunctionGrounder>& get_function_grounder() const;
    const std::shared_ptr<ActionGrounder>& get_action_grounder() const;
    const std::shared_ptr<AxiomGrounder>& get_axiom_grounder() const;
};

}  // namespace mimir

#endif
