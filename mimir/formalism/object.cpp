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
#include "object.hpp"

namespace formalism
{
    ObjectImpl::ObjectImpl(const uint32_t id, const std::string& name, const formalism::Type type) : hash_(0), id(id), name(name), type(type) {}

    bool ObjectImpl::is_free_variable() const { return (name.size() > 0) && (name.at(0) == '?'); }

    bool ObjectImpl::is_constant() const { return !is_free_variable(); }

    Object create_object(const uint32_t id, const std::string& name, const formalism::Type type) { return std::make_shared<ObjectImpl>(id, name, type); }

    formalism::ObjectList filter(const formalism::ObjectList& object_list, const formalism::Type& type)
    {
        formalism::ObjectList filtered_objects;

        for (const auto& obj : object_list)
        {
            if (obj->type == type)
            {
                filtered_objects.push_back(obj);
            }
        }

        return filtered_objects;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ObjectImpl& object) { return os << object.name; }

    std::ostream& operator<<(std::ostream& os, const formalism::Object& object)
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

    std::ostream& operator<<(std::ostream& os, const formalism::ObjectList& objects)
    {
        print_vector(os, objects);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Object>::operator()(const formalism::Object& object) const
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

    std::size_t hash<formalism::ObjectList>::operator()(const formalism::ObjectList& objects) const { return hash_vector<formalism::Object>(objects); }

    bool less<formalism::Object>::operator()(const formalism::Object& left_object, const formalism::Object& right_object) const
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

    bool equal_to<formalism::Object>::operator()(const formalism::Object& left_object, const formalism::Object& right_object) const
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
