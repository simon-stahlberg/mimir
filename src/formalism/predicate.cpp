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
#include "../../include/mimir/formalism/predicate.hpp"
#include "help_functions.hpp"

#include <string>

namespace mimir::formalism
{
    PredicateImpl::PredicateImpl(const uint32_t id, const std::string& name, const mimir::formalism::ObjectList& parameters) :
        hash_(0),
        id(id),
        name(name),
        parameters(parameters),
        arity(parameters.size())
    {
    }

    Predicate create_predicate(const uint32_t id, const std::string& name, const mimir::formalism::ObjectList& parameters)
    {
        return std::make_shared<mimir::formalism::PredicateImpl>(id, name, parameters);
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Predicate& predicate)
    {
        os << predicate->name << "(";

        for (uint32_t index = 0; index < predicate->parameters.size(); ++index)
        {
            const auto object = predicate->parameters.at(index);
            os << object->name;

            if ((index + 1) < predicate->parameters.size())
            {
                os << ", ";
            }
        }

        os << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::PredicateList& predicates)
    {
        print_vector<mimir::formalism::Predicate>(os, predicates);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& predicate) const
    {
        if (!predicate)
        {
            return 0;
        }

        if (!predicate->hash_)
        {
            predicate->hash_ = hash_combine(predicate->id, predicate->name, predicate->parameters);
        }

        return predicate->hash_;
    }

    std::size_t hash<mimir::formalism::PredicateList>::operator()(const mimir::formalism::PredicateList& predicates) const { return hash_vector(predicates); }

    bool less<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& left_predicate,
                                                       const mimir::formalism::Predicate& right_predicate) const
    {
        return less_combine(std::make_tuple(left_predicate->id, left_predicate->name, left_predicate->parameters),
                            std::make_tuple(right_predicate->id, right_predicate->name, right_predicate->parameters));
    }

    bool equal_to<mimir::formalism::Predicate>::operator()(const mimir::formalism::Predicate& left_predicate,
                                                           const mimir::formalism::Predicate& right_predicate) const
    {
        if (left_predicate == right_predicate)
        {
            return true;
        }

        if (!left_predicate || !right_predicate)
        {
            return false;
        }

        const std::hash<mimir::formalism::Predicate> hash;

        if (hash(left_predicate) != hash(right_predicate))
        {
            return false;
        }

        return equal_to_combine(std::make_tuple(left_predicate->id, left_predicate->name, left_predicate->parameters),
                                std::make_tuple(right_predicate->id, right_predicate->name, right_predicate->parameters));
    }
}  // namespace std
