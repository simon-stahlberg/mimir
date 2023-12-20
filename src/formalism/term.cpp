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

#include "../../include/mimir/formalism/term.hpp"
#include "help_functions.hpp"

namespace mimir::formalism
{
    Term::Term(loki::pddl::Variable variable) : external_parameter_(variable), external_object_(nullptr) {}
    Term::Term(loki::pddl::Object object) : external_parameter_(nullptr), external_object_(object) {}

    bool Term::is_free_variable() const { return external_parameter_ != nullptr; }
    bool Term::is_constant() const { return external_object_ != nullptr; }

    std::size_t Term::hash() const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const Term& term)
    {  //
        if (term.external_parameter_)
        {
            return os << "<Variable: \"" << term.external_parameter_->get_name() << "\">";
        }
        else if (term.external_object_)
        {
            return os << "<Object: \"" << term.external_object_->get_name() << "\">";
        }
        else
        {
            return os << "<Term: Invalid>";
        }
    }

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Term>::operator()(const mimir::formalism::Term& term) const { return term.hash(); }

    std::size_t hash<mimir::formalism::TermList>::operator()(const mimir::formalism::TermList& objects) const
    {
        return hash_vector<mimir::formalism::Term>(objects);
    }

    bool less<mimir::formalism::Term>::operator()(const mimir::formalism::Term& left_term, const mimir::formalism::Term& right_term) const
    {
        return left_term < right_term;
    }

    bool equal_to<mimir::formalism::Term>::operator()(const mimir::formalism::Term& left_term, const mimir::formalism::Term& right_term) const
    {
        return left_term == right_term;
    }
}  // namespace std
