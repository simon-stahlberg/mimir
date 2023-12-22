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

#include "../../include/mimir/formalism/predicate.hpp"
#include "../../include/mimir/formalism/term.hpp"
#include "help_functions.hpp"

#include <string>

namespace mimir::formalism
{
    Predicate::Predicate(loki::pddl::Predicate external_predicate) : external_(external_predicate) {}

    const std::string& Predicate::get_name() const { throw std::runtime_error("not implemented"); }
    uint32_t Predicate::get_id() const { throw std::runtime_error("not implemented"); }
    TermList Predicate::get_parameters() const { throw std::runtime_error("not implemented"); }
    std::size_t Predicate::get_arity() const { return get_parameters().size(); }
    std::size_t Predicate::hash() const { throw std::runtime_error("not implemented"); }

    bool Predicate::operator<(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator>(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator==(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator!=(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator<=(const Predicate& other) const { throw std::runtime_error("not implemented"); }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Predicate& predicate)
    {
        os << predicate.get_name() << "(";

        for (uint32_t index = 0; index < predicate.get_parameters().size(); ++index)
        {
            const auto parameter = predicate.get_parameters().at(index);
            os << parameter.get_name();

            if ((index + 1) < predicate.get_parameters().size())
            {
                os << ", ";
            }
        }

        os << ")";
        return os;
    }

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& predicate) const
    {
        throw std::runtime_error("not implemented");
    }

    std::size_t hash<mimir::formalism::PredicateList>::operator()(const mimir::formalism::PredicateList& predicates) const { return hash_vector(predicates); }

    bool less<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& left_predicate,
                                                       const mimir::formalism::Predicate& right_predicate) const
    {
        throw std::runtime_error("not implemented");
    }

    bool equal_to<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& left_predicate,
                                                           const mimir::formalism::Predicate& right_predicate) const
    {
        throw std::runtime_error("not implemented");
    }
}  // namespace std
