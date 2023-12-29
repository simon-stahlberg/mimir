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
#include "help_functions.hpp"

#include <algorithm>

namespace mimir::formalism
{
    Atom::Atom(loki::pddl::Atom external_atom) : external_(external_atom) {}

    // mimir::formalism::Atom ground_predicate(const mimir::formalism::Predicate& predicate, const mimir::formalism::ParameterAssignment& assignment)
    // {
    //     mimir::formalism::ObjectList arguments;

    //     for (const auto& parameter : predicate->parameters)
    //     {
    //         arguments.push_back(assignment.at(parameter));
    //     }

    //     return create_atom(predicate, arguments);
    // }

    uint32_t Atom::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }
    Predicate Atom::get_predicate() const { throw std::invalid_argument("not implemented"); }
    TermList Atom::get_terms() const { throw std::invalid_argument("not implemented"); }

    bool Atom::operator<(const Atom& other) const { throw std::runtime_error("not implemented"); }
    bool Atom::operator>(const Atom& other) const { throw std::runtime_error("not implemented"); }
    bool Atom::operator==(const Atom& other) const { throw std::runtime_error("not implemented"); }
    bool Atom::operator!=(const Atom& other) const { throw std::runtime_error("not implemented"); }
    bool Atom::operator<=(const Atom& other) const { throw std::runtime_error("not implemented"); }

    bool Atom::matches(const Atom& first_atom, const Atom& second_atom) const { throw std::runtime_error("not implemented"); }

    std::size_t Atom::hash() const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Atom& atom)
    {
        os << atom.get_predicate().get_name() << "(";

        for (uint32_t argument_index = 0; argument_index < atom.get_terms().size(); ++argument_index)
        {
            const auto term = atom.get_terms().at(argument_index);
            os << term.get_name();

            if ((argument_index + 1) < atom.get_terms().size())
            {
                os << ", ";
            }
        }

        os << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::AtomList& atoms)
    {
        print_vector<mimir::formalism::Atom>(os, atoms);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& atom) const { throw std::runtime_error("not implemented"); }

    std::size_t hash<mimir::formalism::AtomList>::operator()(const mimir::formalism::AtomList& atoms) const { return hash_vector(atoms); }

    bool less<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const
    {
        return less_combine(std::make_tuple(left_atom.get_predicate(), left_atom.get_terms()),
                            std::make_tuple(right_atom.get_predicate(), right_atom.get_terms()));
    }

    bool equal_to<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const
    {
        throw std::runtime_error("not implemented");
    }
}  // namespace std
