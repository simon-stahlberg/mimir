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

#include <mimir/formalism/type.hpp>

#include <memory>

namespace mimir::formalism
{
    Type::Type(loki::pddl::Type external_type) : external_(external_type) {}

    uint32_t Type::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }

    const std::string& Type::get_name() const { return external_->get_name(); }

    TypeList Type::get_bases() const { throw std::runtime_error("not implemented"); }

    bool is_subtype_of(const mimir::formalism::Type& type, const mimir::formalism::Type& base_type) { throw std::runtime_error("not implemented"); }

    bool Type::operator<(const Type& other) const { throw std::runtime_error("not implemented"); }
    bool Type::operator>(const Type& other) const { throw std::runtime_error("not implemented"); }
    bool Type::operator==(const Type& other) const { throw std::runtime_error("not implemented"); }
    bool Type::operator!=(const Type& other) const { throw std::runtime_error("not implemented"); }
    bool Type::operator<=(const Type& other) const { throw std::runtime_error("not implemented"); }

    std::size_t Type::hash() const { throw std::runtime_error("not implemented"); }

}  // namespace mimir::formalism

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Type>::operator()(const mimir::formalism::Type& type) const { return type.hash(); }

    std::size_t hash<mimir::formalism::TypeList>::operator()(const mimir::formalism::TypeList& types) const { return hash_vector(types); }

    bool less<mimir::formalism::Type>::operator()(const mimir::formalism::Type& left_type, const mimir::formalism::Type& right_type) const
    {
        return left_type < right_type;
    }

    bool equal_to<mimir::formalism::Type>::operator()(const mimir::formalism::Type& left_type, const mimir::formalism::Type& right_type) const
    {
        return left_type == right_type;
    }
}  // namespace std
*/
