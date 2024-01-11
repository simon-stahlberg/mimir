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

#include "help_functions.hpp"

#include <mimir/formalism/atom.hpp>
#include <mimir/formalism/literal.hpp>
#include <mimir/formalism/problem.hpp>
#include <mimir/formalism/term.hpp>
#include <mimir/generators/lifted_successor_generator.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <loki/domain/parser.hpp>
#include <loki/problem/parser.hpp>

namespace mimir::formalism
{
    Problem::Problem(loki::pddl::Problem external_problem) : external_(std::move(external_problem)) {}

    Problem Problem::parse(const std::string& domain_path, const std::string& problem_path)
    {
        loki::DomainParser domain_parser(domain_path);
        loki::ProblemParser problem_parser(problem_path, domain_parser);
        return Problem(problem_parser.get_problem());
    }

    ProblemList Problem::parse(const std::string& domain_path, const std::vector<std::string>& problem_paths)
    {
        ProblemList problems;
        loki::DomainParser domain_parser(domain_path);

        for (const auto& path : problem_paths)
        {
            loki::ProblemParser problem_parser(path, domain_parser);
            problems.emplace_back(Problem(problem_parser.get_problem()));
        }

        return problems;
    }

    uint32_t Problem::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }

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

    const AtomSet& Problem::get_static_atoms() const { throw std::runtime_error("not implemented"); }

    std::size_t Problem::hash() const { throw std::runtime_error("not implemented"); }

    bool Problem::operator<(const Problem& other) const { throw std::runtime_error("not implemented"); }

    bool Problem::operator>(const Problem& other) const { throw std::runtime_error("not implemented"); }

    bool Problem::operator==(const Problem& other) const { throw std::runtime_error("not implemented"); }

    bool Problem::operator!=(const Problem& other) const { throw std::runtime_error("not implemented"); }

    bool Problem::operator<=(const Problem& other) const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ProblemList& problems)
    {
        print_vector(os, problems);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& problem) const { return problem.hash(); }

    bool less<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& left_problem, const mimir::formalism::Problem& right_problem) const
    {
        return left_problem < right_problem;
    }

    bool equal_to<mimir::formalism::Problem>::operator()(const mimir::formalism::Problem& left_problem, const mimir::formalism::Problem& right_problem) const
    {
        return left_problem == right_problem;
    }
}  // namespace std
