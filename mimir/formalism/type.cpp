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
#include "type.hpp"

#include <memory>

namespace formalism
{
    TypeImpl::TypeImpl(const std::string& name, formalism::Type base) : name(name), base(base) {}

    bool is_subtype_of(const formalism::Type& type, const formalism::Type& base_type)
    {
        formalism::Type current = type;

        while (current != nullptr)
        {
            if (current == base_type)
            {
                return true;
            }

            current = current->base;
        }

        return false;
    }

    Type create_type(const std::string& name, formalism::Type base) { return std::make_shared<TypeImpl>(name, base); }

    std::ostream& operator<<(std::ostream& os, const formalism::Type& type)
    {
        os << type->name;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::TypeList& types)
    {
        print_vector(os, types);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Type>::operator()(const formalism::Type& type) const
    {
        std::hash<std::string> hash_name;
        return hash_name(type->name);
    }

    std::size_t hash<formalism::TypeList>::operator()(const formalism::TypeList& types) const { return hash_vector(types); }

    bool less<formalism::Type>::operator()(const formalism::Type& left_type, const formalism::Type& right_type) const
    {
        const std::less<std::string> less_name;
        return less_name(left_type->name, right_type->name);
    }

    bool equal_to<formalism::Type>::operator()(const formalism::Type& left_type, const formalism::Type& right_type) const
    {
        const std::equal_to<std::string> equal_to_name;
        return equal_to_name(left_type->name, right_type->name);
    }
}  // namespace std
