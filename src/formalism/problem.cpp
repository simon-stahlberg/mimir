/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../include/mimir/formalism/atom.hpp"
#include "../../include/mimir/formalism/literal.hpp"
#include "../../include/mimir/formalism/problem.hpp"
#include "../../include/mimir/formalism/term.hpp"
#include "../../include/mimir/generators/lifted_successor_generator.hpp"
#include "help_functions.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace mimir::formalism
{
    Problem::Problem(loki::pddl::Problem external_problem) : external_(std::move(external_problem)) {}

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Problem& problem)
    {
        os << "Name: " << problem.get_name() << std::endl;
        // os << "Objects: " << problem.get_objects() << std::endl;
        // os << "Goal: " << problem.get_goal_literals() << std::endl;
        // os << "Initial: " << problem.get_initial_atoms() << std::endl;
        return os;
    }

    Domain Problem::get_domain() const { throw std::runtime_error("not implemented"); }

    const std::string& Problem::get_name() const { throw std::runtime_error("not implemented"); }

    TermList Problem::get_objects() const { throw std::runtime_error("not implemented"); }

    AtomList Problem::get_initial_atoms() const { throw std::runtime_error("not implemented"); }

    LiteralList Problem::get_goal_literals() const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ProblemList& problems)
    {
        print_vector(os, problems);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& problem) const
    {
        throw std::runtime_error("not implemented");
        // return hash_combine(problem->name, problem->domain, problem->objects, problem->initial, problem->goal);
    }

    bool less<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& left_problem, const mimir::formalism::Problem& right_problem) const
    {
        throw std::runtime_error("not implemented");
        // return less_combine(std::make_tuple(left_problem->name, left_problem->domain, left_problem->objects, left_problem->initial, left_problem->goal),
        //                     std::make_tuple(right_problem->name, right_problem->domain, right_problem->objects, right_problem->initial,
        //                     right_problem->goal));
    }

    bool equal_to<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& left_problem, const mimir::formalism::Problem& right_problem) const
    {
        throw std::runtime_error("not implemented");
        // if (left_problem.get() == right_problem.get())
        // {
        //     return true;
        // }

        // return equal_to_combine(
        //     std::make_tuple(left_problem->name, left_problem->domain, left_problem->objects, left_problem->initial, left_problem->goal),
        //     std::make_tuple(right_problem->name, right_problem->domain, right_problem->objects, right_problem->initial, right_problem->goal));
    }
}  // namespace std
