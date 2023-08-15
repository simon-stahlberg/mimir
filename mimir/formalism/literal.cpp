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
#include "literal.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace formalism
{
    LiteralImpl::LiteralImpl(const Atom& atom, const bool negated) : hash_(0), atom(atom), negated(negated) {}

    formalism::Literal create_literal(const Atom& atom, const bool negated) { return std::make_shared<LiteralImpl>(atom, negated); }

    formalism::Literal ground_literal(const formalism::Literal& literal, const formalism::ParameterAssignment& assignment)
    {
        // Note: the assignment might not be complete, in which case a partially ground literal is returned.

        formalism::ObjectList arguments;
        arguments.reserve(literal->atom->arguments.size());

        for (const auto& parameter : literal->atom->arguments)
        {
            if (parameter->is_free_variable())
            {
                const auto assignment_handler = assignment.find(parameter);

                if (assignment_handler != assignment.end())
                {
                    arguments.emplace_back(assignment_handler->second);
                }
                else
                {
                    arguments.emplace_back(parameter);
                }
            }
            else
            {
                arguments.emplace_back(parameter);
            }
        }

        const auto ground_atom = formalism::create_atom(literal->atom->predicate, arguments);
        return formalism::create_literal(ground_atom, literal->negated);
    }

    formalism::LiteralList ground_literal_list(const formalism::LiteralList& literal_list, const formalism::ParameterAssignment& assignment)
    {
        formalism::LiteralList ground_literal_list;

        for (const auto& literal : literal_list)
        {
            ground_literal_list.push_back(formalism::ground_literal(literal, assignment));
        }

        return ground_literal_list;
    }

    formalism::AtomList as_atoms(const formalism::LiteralList& literals)
    {
        formalism::AtomList atoms;

        for (const auto& literal : literals)
        {
            if (literal->negated)
            {
                throw std::invalid_argument("literal is negated");
            }
            else
            {
                atoms.push_back(literal->atom);
            }
        }

        return atoms;
    }

    bool contains_predicate(const formalism::LiteralList& literals, const formalism::Predicate& predicate)
    {
        for (const auto& literal : literals)
        {
            if (literal->atom->predicate == predicate)
            {
                return true;
            }
        }

        return false;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::Literal& literal)
    {
        if (literal->negated)
        {
            os << "-";
        }

        os << literal->atom;

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::LiteralList& literals)
    {
        print_vector<formalism::Literal>(os, literals);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Literal>::operator()(const formalism::Literal& literal) const
    {
        if (!literal)
        {
            return 0;
        }

        if (!literal->hash_)
        {
            literal->hash_ = hash_combine(literal->atom, literal->negated);
        }

        return literal->hash_;
    }

    std::size_t hash<formalism::LiteralList>::operator()(const formalism::LiteralList& literals) const { return hash_vector<formalism::Literal>(literals); }

    bool less<formalism::Literal>::operator()(const formalism::Literal& left_literal, const formalism::Literal& right_literal) const
    {
        return less_combine(std::make_tuple(left_literal->atom, left_literal->negated), std::make_tuple(right_literal->atom, right_literal->negated));
    }

    bool equal_to<formalism::Literal>::operator()(const formalism::Literal& left_literal, const formalism::Literal& right_literal) const
    {
        if (left_literal == right_literal)
        {
            return true;
        }

        if (!left_literal || !right_literal)
        {
            return false;
        }

        const std::hash<formalism::Literal> hash;

        if (hash(left_literal) != hash(right_literal))
        {
            return false;
        }

        if (left_literal->negated != right_literal->negated)
        {
            return false;
        }

        return std::equal_to<formalism::Atom>()(left_literal->atom, right_literal->atom);
    }
}  // namespace std
