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

#include <mimir/formalism/atom.hpp>
#include <mimir/formalism/literal.hpp>
#include <mimir/formalism/problem.hpp>
#include <mimir/formalism/term.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <loki/domain/parser.hpp>
#include <loki/problem/parser.hpp>

namespace mimir
{
    ProblemImpl::ProblemImpl(int identifier, loki::pddl::Problem external_problem) : external_(external_problem) {}

    uint32_t ProblemImpl::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }

    Domain ProblemImpl::get_domain() const { throw std::runtime_error("not implemented"); }

    const std::string& ProblemImpl::get_name() const { throw std::runtime_error("not implemented"); }

    TermList ProblemImpl::get_objects() const { throw std::runtime_error("not implemented"); }

    AtomList ProblemImpl::get_initial_atoms() const { throw std::runtime_error("not implemented"); }

    LiteralList ProblemImpl::get_goal_literals() const { throw std::runtime_error("not implemented"); }

    const AtomSet& ProblemImpl::get_static_atoms() const { throw std::runtime_error("not implemented"); }

    std::size_t ProblemImpl::hash() const { throw std::runtime_error("not implemented"); }

    bool ProblemImpl::operator<(const ProblemImpl& other) const { throw std::runtime_error("not implemented"); }

    bool ProblemImpl::operator>(const ProblemImpl& other) const { throw std::runtime_error("not implemented"); }

    bool ProblemImpl::operator==(const ProblemImpl& other) const { throw std::runtime_error("not implemented"); }

    bool ProblemImpl::operator!=(const ProblemImpl& other) const { throw std::runtime_error("not implemented"); }

    bool ProblemImpl::operator<=(const ProblemImpl& other) const { throw std::runtime_error("not implemented"); }

}  // namespace mimir

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::ProblemImpl>::operator()(const mimir::ProblemImpl& problem) const { return problem.hash(); }

    bool less<mimir::ProblemImpl>::operator()(const mimir::ProblemImpl& left_problem, const mimir::ProblemImpl& right_problem) const
    {
        return left_problem < right_problem;
    }

    bool equal_to<mimir::ProblemImpl>::operator()(const mimir::ProblemImpl& left_problem, const mimir::ProblemImpl& right_problem) const
    {
        return left_problem == right_problem;
    }
}  // namespace std
*/
