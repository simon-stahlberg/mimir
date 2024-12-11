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

#include "mimir/search/grounders/function_grounder.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/grounders/variable_grounder.hpp"

namespace mimir
{

FunctionGrounder::FunctionGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories))
{
}

GroundFunction FunctionGrounder::ground_function(Function function, const ObjectList& binding)
{
    auto grounded_terms = ObjectList {};
    ground_variables(function->get_terms(), binding, grounded_terms);
    return m_pddl_repositories->get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);
}

Problem FunctionGrounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& FunctionGrounder::get_pddl_repositories() const { return m_pddl_repositories; }

}