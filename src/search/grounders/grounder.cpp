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

#include "mimir/search/grounders/grounder.hpp"

namespace mimir
{
Grounder::Grounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_literal_grounder(std::make_shared<LiteralGrounder>(m_problem, m_pddl_repositories)),
    m_function_grounder(std::make_shared<FunctionGrounder>(m_problem, m_pddl_repositories)),
    m_action_grounder(std::make_shared<ActionGrounder>(m_literal_grounder, m_function_grounder)),
    m_axiom_grounder(std::make_shared<AxiomGrounder>(m_literal_grounder))
{
}

Problem Grounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& Grounder::get_pddl_repositories() const { return m_pddl_repositories; }

const std::shared_ptr<LiteralGrounder>& Grounder::get_literal_grounder() const { return m_literal_grounder; }

const std::shared_ptr<FunctionGrounder>& Grounder::get_function_grounder() const { return m_function_grounder; }

const std::shared_ptr<ActionGrounder>& Grounder::get_action_grounder() const { return m_action_grounder; }

const std::shared_ptr<AxiomGrounder>& Grounder::get_axiom_grounder() const { return m_axiom_grounder; }

}