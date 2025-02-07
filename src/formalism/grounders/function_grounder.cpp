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

#include "mimir/formalism/grounders/function_grounder.hpp"

#include "mimir/formalism/grounders/variable_grounder.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"

namespace mimir
{

FunctionGrounder::FunctionGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories))
{
}

template<FunctionTag F>
GroundFunction<F> FunctionGrounder::ground(Function<F> function, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_grounding_tables, boost::hana::type<GroundFunction<F>> {});

    /* 2. Access the context-independent function grounding table */
    const auto function_skeleton_index = function->get_function_skeleton()->get_index();
    if (function_skeleton_index >= grounding_tables.size())
    {
        grounding_tables.resize(function_skeleton_index + 1);
    }

    auto& grounding_table = grounding_tables.at(function_skeleton_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    // Note: this is important and saves a lot of memory.
    auto grounded_terms = ObjectList {};
    ground_variables(function->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function */

    const auto grounded_function = m_pddl_repositories->get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundFunction<F>(grounded_function));

    /* 6. Return the resulting ground literal */

    return grounded_function;
}

template GroundFunction<Static> FunctionGrounder::ground(Function<Static> function, const ObjectList& binding);
template GroundFunction<Fluent> FunctionGrounder::ground(Function<Fluent> function, const ObjectList& binding);
template GroundFunction<Auxiliary> FunctionGrounder::ground(Function<Auxiliary> function, const ObjectList& binding);

Problem FunctionGrounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& FunctionGrounder::get_pddl_repositories() const { return m_pddl_repositories; }

}