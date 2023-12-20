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
#include "help_functions.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace mimir::formalism
{
    Literal::Literal(loki::pddl::Literal external_literal) : external_(external_literal) {}

    bool Literal::is_negated() const { return external_->is_negated(); }

    Atom Literal::get_atom() const { return Atom(external_->get_atom()); }

    Literal ground_literal(const Literal& literal, const ParameterAssignment& assignment) { throw std::runtime_error("not implemented"); }

    LiteralList ground_literal_list(const LiteralList& literal_list, const ParameterAssignment& assignment)
    {
        LiteralList ground_literal_list;

        for (const auto& literal : literal_list)
        {
            ground_literal_list.push_back(ground_literal(literal, assignment));
        }

        return ground_literal_list;
    }

    AtomList as_atoms(const LiteralList& literals) { throw std::runtime_error("not implemented"); }

    bool contains_predicate(const LiteralList& literals, const Predicate& predicate) { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const Literal& literal)
    {
        if (literal.is_negated())
        {
            os << "-";
        }

        os << literal.get_atom();

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const LiteralList& literals)
    {
        print_vector<Literal>(os, literals);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Literal>::operator()(const mimir::formalism::Literal& literal) const { return literal.hash(); }

    std::size_t hash<mimir::formalism::LiteralList>::operator()(const mimir::formalism::LiteralList& literals) const
    {
        return hash_vector<mimir::formalism::Literal>(literals);
    }

    bool less<mimir::formalism::Literal>::operator()(const mimir::formalism::Literal& left_literal, const mimir::formalism::Literal& right_literal) const
    {
        return less_combine(std::make_tuple(left_literal.get_atom(), left_literal.is_negated()),
                            std::make_tuple(right_literal.get_atom(), right_literal.is_negated()));
    }

    bool equal_to<mimir::formalism::Literal>::operator()(const mimir::formalism::Literal& left_literal, const mimir::formalism::Literal& right_literal) const
    {
        const std::hash<mimir::formalism::Literal> hash;

        if (hash(left_literal) != hash(right_literal))
        {
            return false;
        }

        if (left_literal.is_negated() != right_literal.is_negated())
        {
            return false;
        }

        return std::equal_to<mimir::formalism::Atom>()(left_literal.get_atom(), right_literal.get_atom());
    }
}  // namespace std
