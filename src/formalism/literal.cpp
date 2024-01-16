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
#include <mimir/formalism/predicate.hpp>
#include <mimir/formalism/term.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

namespace mimir
{
    /*
     * Member functions
     */

    Literal::Literal(loki::pddl::Literal external_literal) : external_(external_literal) {}

    uint32_t Literal::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }

    bool Literal::is_negated() const { return external_->is_negated(); }

    Atom Literal::get_atom() const { return Atom(external_->get_atom()); }

    uint32_t Literal::get_atom_id() const { throw std::runtime_error("not implemented"); }

    Predicate Literal::get_predicate() const { return Predicate(external_->get_atom()->get_predicate()); }

    Literal Literal::ground_literal(const ParameterAssignment& assignment) const { throw std::runtime_error("not implemented"); }

    bool Literal::contains(const Predicate& predicate) const { throw std::runtime_error("not implemented"); }

    std::size_t Literal::hash() const { throw std::runtime_error("not implemented"); }

    bool Literal::operator<(const Literal& other) const { throw std::runtime_error("not implemented"); }

    bool Literal::operator>(const Literal& other) const { throw std::runtime_error("not implemented"); }

    bool Literal::operator==(const Literal& other) const { throw std::runtime_error("not implemented"); }

    bool Literal::operator!=(const Literal& other) const { throw std::runtime_error("not implemented"); }

    bool Literal::operator<=(const Literal& other) const { throw std::runtime_error("not implemented"); }

    /*
     * Namespace functions
     */

    AtomList as_atoms(const LiteralList& literals) { throw std::runtime_error("not implemented"); }

    LiteralList ground_literals(const LiteralList& literal_list, const ParameterAssignment& assignment)
    {
        LiteralList ground_literal_list;

        for (const auto& literal : literal_list)
        {
            ground_literal_list.push_back(literal.ground_literal(assignment));
        }

        return ground_literal_list;
    }

}  // namespace mimir

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::Literal>::operator()(const mimir::Literal& literal) const { return literal.hash(); }

    std::size_t hash<mimir::LiteralList>::operator()(const mimir::LiteralList& literals) const
    {
        return hash_vector<mimir::Literal>(literals);
    }

    bool less<mimir::Literal>::operator()(const mimir::Literal& left_literal, const mimir::Literal& right_literal) const
    {
        return left_literal < right_literal;
    }

    bool equal_to<mimir::Literal>::operator()(const mimir::Literal& left_literal, const mimir::Literal& right_literal) const
    {
        return left_literal == right_literal;
    }
}  // namespace std
*/
