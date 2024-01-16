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

#include <mimir/formalism/predicate.hpp>

#include <string>

namespace mimir
{
    Predicate::Predicate(loki::pddl::Predicate external_predicate) : external_(external_predicate) {}

    uint32_t Predicate::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }
    const std::string& Predicate::get_name() const { throw std::runtime_error("not implemented"); }
    TermList Predicate::get_parameters() const { throw std::runtime_error("not implemented"); }
    std::size_t Predicate::get_arity() const { return get_parameters().size(); }
    std::size_t Predicate::hash() const { throw std::runtime_error("not implemented"); }

    bool Predicate::operator<(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator>(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator==(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator!=(const Predicate& other) const { throw std::runtime_error("not implemented"); }
    bool Predicate::operator<=(const Predicate& other) const { throw std::runtime_error("not implemented"); }

}  // namespace mimir

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::Predicate>::operator()(const mimir::Predicate& predicate) const
    {
        throw std::runtime_error("not implemented");
    }

    std::size_t hash<mimir::PredicateList>::operator()(const mimir::PredicateList& predicates) const { return hash_vector(predicates); }

    bool less<mimir::Predicate>::operator()(const mimir::Predicate& left_predicate,
                                                       const mimir::Predicate& right_predicate) const
    {
        throw std::runtime_error("not implemented");
    }

    bool equal_to<mimir::Predicate>::operator()(const mimir::Predicate& left_predicate,
                                                           const mimir::Predicate& right_predicate) const
    {
        throw std::runtime_error("not implemented");
    }
}  // namespace std
*/
