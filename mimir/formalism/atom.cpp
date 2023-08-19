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


#include "atom.hpp"
#include "help_functions.hpp"

#include <algorithm>

namespace formalism
{
    AtomImpl::AtomImpl(const formalism::Predicate& predicate, formalism::ObjectList&& arguments) :
        hash_(0),
        predicate(predicate),
        arguments(std::move(arguments))
    {
    }

    AtomImpl::AtomImpl(const formalism::Predicate& predicate, const formalism::ObjectList& arguments) : hash_(0), predicate(predicate), arguments(arguments) {}

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

    formalism::Atom ground_predicate(const formalism::Predicate& predicate, const formalism::ParameterAssignment& assignment)
    {
        formalism::ObjectList arguments;

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

    Atom replace_term(const Atom& atom, uint32_t index, const formalism::Object& object)
    {
        auto terms = atom->arguments;

        if (index >= terms.size())
        {
            throw std::invalid_argument("index is out of range");
        }

        terms[index] = object;

        return create_atom(atom->predicate, terms);
    }

    Atom create_atom(const formalism::Predicate& predicate, formalism::ObjectList&& arguments)
    {
        return std::make_shared<formalism::AtomImpl>(predicate, std::move(arguments));
    }

    Atom create_atom(const formalism::Predicate& predicate, const formalism::ObjectList& arguments)
    {
        return std::make_shared<formalism::AtomImpl>(predicate, arguments);
    }

    formalism::AtomList filter(const formalism::AtomList& atom_list, const formalism::Object& obj, int32_t argument_index)
    {
        formalism::AtomList filtered_atom_list;

        for (const auto& atom : atom_list)
        {
            if (atom->arguments.at(argument_index) == obj)
            {
                filtered_atom_list.push_back(atom);
            }
        }

        return filtered_atom_list;
    }

    formalism::AtomList filter(const formalism::AtomList& atom_list, const formalism::ObjectList& object_list, int32_t argument_index)
    {
        formalism::AtomSet filtered_atom_set;

        for (const auto& atom : atom_list)
        {
            if (std::count(object_list.begin(), object_list.end(), atom->arguments.at(argument_index)))
            {
                filtered_atom_set.insert(atom);
            }
        }

        return formalism::AtomList(filtered_atom_set.begin(), filtered_atom_set.end());
    }

    formalism::AtomList exclude(const formalism::AtomList& atom_list, const formalism::AtomList& other_list)
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

    formalism::ObjectList get_objects(const formalism::AtomList& atom_list, int32_t argument_index)
    {
        formalism::ObjectList object_list;

        for (const auto& atom : atom_list)
        {
            object_list.push_back(atom->arguments.at(argument_index));
        }

        return object_list;
    }

    formalism::ObjectList get_unique_objects(const formalism::AtomList& atom_list, int32_t argument_index)
    {
        std::unordered_set<formalism::Object> object_set;

        for (const auto& atom : atom_list)
        {
            object_set.insert(atom->arguments.at(argument_index));
        }

        return formalism::ObjectList(object_set.begin(), object_set.end());
    }

    formalism::ObjectList concatenate(const formalism::ObjectList& left_list, const formalism::ObjectList& right_list)
    {
        auto concatenated = left_list;
        concatenated.insert(concatenated.end(), right_list.begin(), left_list.end());
        return concatenated;
    }

    formalism::ObjectList difference(const formalism::ObjectList& left_list, const formalism::ObjectList& right_list)
    {
        formalism::ObjectList sorted_right_list = right_list;
        std::sort(sorted_right_list.begin(), sorted_right_list.end());

        formalism::ObjectList difference;

        for (const auto& obj : left_list)
        {
            if (!std::binary_search(sorted_right_list.begin(), sorted_right_list.end(), obj))
            {
                difference.push_back(obj);
            }
        }

        return difference;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::Atom& atom)
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

    std::ostream& operator<<(std::ostream& os, const formalism::AtomList& atoms)
    {
        print_vector<formalism::Atom>(os, atoms);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Atom>::operator()(const formalism::Atom& atom) const
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

    std::size_t hash<formalism::AtomList>::operator()(const formalism::AtomList& atoms) const { return hash_vector(atoms); }

    bool less<formalism::Atom>::operator()(const formalism::Atom& left_atom, const formalism::Atom& right_atom) const
    {
        return less_combine(std::make_tuple(left_atom->predicate, left_atom->arguments), std::make_tuple(right_atom->predicate, right_atom->arguments));
    }

    bool equal_to<formalism::Atom>::operator()(const formalism::Atom& left_atom, const formalism::Atom& right_atom) const
    {
        if (left_atom == right_atom)
        {
            return true;
        }

        if (!left_atom || !right_atom)
        {
            return false;
        }

        const std::hash<formalism::Atom> hash;

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
