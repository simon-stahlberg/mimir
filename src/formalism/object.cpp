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

#include "../../include/mimir/formalism/object.hpp"
#include "help_functions.hpp"

namespace mimir::formalism
{
    ObjectImpl::ObjectImpl(const uint32_t id, const std::string& name, const mimir::formalism::Type type) : hash_(0), id(id), name(name), type(type) {}

    bool ObjectImpl::is_free_variable() const { return (name.size() > 0) && (name.at(0) == '?'); }

    bool ObjectImpl::is_constant() const { return !is_free_variable(); }

    Object create_object(const uint32_t id, const std::string& name, const mimir::formalism::Type type) { return std::make_shared<ObjectImpl>(id, name, type); }

    mimir::formalism::ObjectList filter(const mimir::formalism::ObjectList& object_list, const mimir::formalism::Type& type)
    {
        mimir::formalism::ObjectList filtered_objects;

        for (const auto& obj : object_list)
        {
            if (obj->type == type)
            {
                filtered_objects.push_back(obj);
            }
        }

        return filtered_objects;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ObjectImpl& object) { return os << object.name; }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Object& object)
    {
        if (object)
        {
            return os << *object;
        }
        else
        {
            return os << "<NULL>";
        }
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ObjectList& objects)
    {
        print_vector(os, objects);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Object>::operator()(const mimir::formalism::Object& object) const
    {
        if (!object)
        {
            return 0;
        }

        if (!object->hash_)
        {
            object->hash_ = hash_combine(object->id, object->name, object->type);
        }

        return object->hash_;
    }

    std::size_t hash<mimir::formalism::ObjectList>::operator()(const mimir::formalism::ObjectList& objects) const
    {
        return hash_vector<mimir::formalism::Object>(objects);
    }

    bool less<mimir::formalism::Object>::operator()(const mimir::formalism::Object& left_object, const mimir::formalism::Object& right_object) const
    {
        if (left_object == right_object)
        {
            return false;
        }

        if (!left_object)
        {
            return true;
        }

        if (!right_object)
        {
            return false;
        }

        return less_combine(std::make_tuple(left_object->id, left_object->name, left_object->type),
                            std::make_tuple(right_object->id, right_object->name, right_object->type));
    }

    bool equal_to<mimir::formalism::Object>::operator()(const mimir::formalism::Object& left_object, const mimir::formalism::Object& right_object) const
    {
        if (left_object == right_object)
        {
            return true;
        }

        if (!left_object || !right_object)
        {
            return false;
        }

        return left_object->id == right_object->id;
    }
}  // namespace std
