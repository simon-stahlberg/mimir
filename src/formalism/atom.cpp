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
    void AtomImpl::validate() const
    {
        if (predicate->arity != static_cast<uint32_t>(arguments.size()))
        {
            throw std::invalid_argument("size of arguments does not match predicate");
        }

        for (uint32_t index = 0; index < predicate->arity; ++index)
        {
            const auto& argument_type = arguments[index]->type;
            const auto& parameter_type = predicate->parameters[index]->type;

            if (!mimir::formalism::is_subtype_of(argument_type, parameter_type))
            {
                const auto error_message = "type mismatch at index " + std::to_string(index) + ": expected \"" + parameter_type->name + "\" but got \""
                                           + argument_type->name + "\" (" + arguments[index]->name + ")";
                throw std::invalid_argument(error_message);
            }
        }
    }

    AtomImpl::AtomImpl(const mimir::formalism::Predicate& predicate, mimir::formalism::ObjectList&& arguments) :
        hash_(0),
        predicate(predicate),
        arguments(std::move(arguments))
    {
        // validate();  // For performance reasons, skip validation for this constructor.
    }

    AtomImpl::AtomImpl(const mimir::formalism::Predicate& predicate, const mimir::formalism::ObjectList& arguments) :
        hash_(0),
        predicate(predicate),
        arguments(arguments)
    {
        validate();
    }

    bool AtomImpl::operator==(const AtomImpl& other) const
    {
        // Assumes pointers to the same predicate and objects

        if (this->predicate != other.predicate)
        {
            return false;
        }

        if (this->arguments.size() != other.arguments.size())
        {
            return false;
        }

        for (uint32_t index = 0; index < this->arguments.size(); ++index)
        {
            const auto& lhs_arg = this->arguments.at(index);
            const auto& rhs_arg = other.arguments.at(index);

            if (lhs_arg != rhs_arg)
            {
                return false;
            }
        }

        return true;
    }

    bool AtomImpl::operator!=(const AtomImpl& other) const { return !(this->operator==(other)); }

    mimir::formalism::Atom ground_predicate(const mimir::formalism::Predicate& predicate, const mimir::formalism::ParameterAssignment& assignment)
    {
        mimir::formalism::ObjectList arguments;

        for (const auto& parameter : predicate->parameters)
        {
            arguments.push_back(assignment.at(parameter));
        }

        return create_atom(predicate, arguments);
    }

    bool matches(const Atom& first_atom, const Atom& second_atom)
    {
        if (first_atom->predicate != second_atom->predicate)
        {
            return false;
        }

        for (uint32_t index = 0; index < first_atom->arguments.size(); ++index)
        {
            const auto& first_argument = first_atom->arguments.at(index);
            const auto& second_argument = second_atom->arguments.at(index);

            if (!first_argument->is_free_variable() && !second_argument->is_free_variable() && (first_argument != second_argument))
            {
                return false;
            }
        }

        return true;
    }

    Atom replace_term(const Atom& atom, uint32_t index, const mimir::formalism::Object& object)
    {
        auto terms = atom->arguments;

        if (index >= terms.size())
        {
            throw std::invalid_argument("index is out of range");
        }

        terms[index] = object;

        return create_atom(atom->predicate, terms);
    }

    Atom create_atom(const mimir::formalism::Predicate& predicate, mimir::formalism::ObjectList&& arguments)
    {
        return std::make_shared<mimir::formalism::AtomImpl>(predicate, std::move(arguments));
    }

    Atom create_atom(const mimir::formalism::Predicate& predicate, const mimir::formalism::ObjectList& arguments)
    {
        return std::make_shared<mimir::formalism::AtomImpl>(predicate, arguments);
    }

    mimir::formalism::AtomList filter(const mimir::formalism::AtomList& atom_list, const mimir::formalism::Object& obj, int32_t argument_index)
    {
        mimir::formalism::AtomList filtered_atom_list;

        for (const auto& atom : atom_list)
        {
            if (atom->arguments.at(argument_index) == obj)
            {
                filtered_atom_list.push_back(atom);
            }
        }

        return filtered_atom_list;
    }

    mimir::formalism::AtomList filter(const mimir::formalism::AtomList& atom_list, const mimir::formalism::ObjectList& object_list, int32_t argument_index)
    {
        mimir::formalism::AtomSet filtered_atom_set;

        for (const auto& atom : atom_list)
        {
            if (std::count(object_list.begin(), object_list.end(), atom->arguments.at(argument_index)))
            {
                filtered_atom_set.insert(atom);
            }
        }

        return mimir::formalism::AtomList(filtered_atom_set.begin(), filtered_atom_set.end());
    }

    mimir::formalism::AtomList exclude(const mimir::formalism::AtomList& atom_list, const mimir::formalism::AtomList& other_list)
    {
        AtomList excluded_atom_list;

        std::equal_to<Atom> equal_to;

        for (const auto& atom : atom_list)
        {
            bool any = false;

            for (const auto& other_atom : other_list)
            {
                if (equal_to(atom, other_atom))
                {
                    any = true;
                    break;
                }
            }

            if (!any)
            {
                excluded_atom_list.push_back(atom);
            }
        }

        return excluded_atom_list;
    }

    mimir::formalism::ObjectList get_objects(const mimir::formalism::AtomList& atom_list, int32_t argument_index)
    {
        mimir::formalism::ObjectList object_list;

        for (const auto& atom : atom_list)
        {
            object_list.push_back(atom->arguments.at(argument_index));
        }

        return object_list;
    }

    mimir::formalism::ObjectList get_unique_objects(const mimir::formalism::AtomList& atom_list, int32_t argument_index)
    {
        mimir::tsl::robin_set<mimir::formalism::Object> object_set;

        for (const auto& atom : atom_list)
        {
            object_set.insert(atom->arguments.at(argument_index));
        }

        return mimir::formalism::ObjectList(object_set.begin(), object_set.end());
    }

    mimir::formalism::ObjectList concatenate(const mimir::formalism::ObjectList& left_list, const mimir::formalism::ObjectList& right_list)
    {
        auto concatenated = left_list;
        concatenated.insert(concatenated.end(), right_list.begin(), left_list.end());
        return concatenated;
    }

    mimir::formalism::ObjectList difference(const mimir::formalism::ObjectList& left_list, const mimir::formalism::ObjectList& right_list)
    {
        mimir::formalism::ObjectList sorted_right_list = right_list;
        std::sort(sorted_right_list.begin(), sorted_right_list.end());

        mimir::formalism::ObjectList difference;

        for (const auto& obj : left_list)
        {
            if (!std::binary_search(sorted_right_list.begin(), sorted_right_list.end(), obj))
            {
                difference.push_back(obj);
            }
        }

        return difference;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Atom& atom)
    {
        os << atom->predicate->name << "(";

        for (uint32_t argument_index = 0; argument_index < atom->arguments.size(); ++argument_index)
        {
            const auto object = atom->arguments.at(argument_index);
            os << object->name;

            if ((argument_index + 1) < atom->arguments.size())
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
    std::size_t hash<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& atom) const
    {
        if (!atom)
        {
            return 0;
        }

        if (!atom->hash_)
        {
            atom->hash_ = hash_combine(atom->predicate, atom->arguments);
        }

        return atom->hash_;
    }

    std::size_t hash<mimir::formalism::AtomList>::operator()(const mimir::formalism::AtomList& atoms) const { return hash_vector(atoms); }

    bool less<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const
    {
        return less_combine(std::make_tuple(left_atom->predicate, left_atom->arguments), std::make_tuple(right_atom->predicate, right_atom->arguments));
    }

    bool equal_to<mimir::formalism::Atom>::operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const
    {
        if (left_atom == right_atom)
        {
            return true;
        }

        if (!left_atom || !right_atom)
        {
            return false;
        }

        const std::hash<mimir::formalism::Atom> hash;

        if (hash(left_atom) != hash(right_atom))
        {
            return false;
        }

        if (left_atom->predicate != right_atom->predicate)
        {
            return false;
        }

        return left_atom->arguments == right_atom->arguments;
    }
}  // namespace std
