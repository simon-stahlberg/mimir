/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#include "mimir/formalism/ground_literal.hpp"

#include "formatter.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{
template<PredicateTag P>
GroundLiteralImpl<P>::GroundLiteralImpl(Index index, bool is_negated, GroundAtom<P> atom) : m_index(index), m_is_negated(is_negated), m_atom(std::move(atom))
{
}

template<PredicateTag P>
Index GroundLiteralImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateTag P>
bool GroundLiteralImpl<P>::is_negated() const
{
    return m_is_negated;
}

template<PredicateTag P>
const GroundAtom<P>& GroundLiteralImpl<P>::get_atom() const
{
    return m_atom;
}

template<PredicateTag P>
Literal<P> GroundLiteralImpl<P>::lift(const TermList& terms, PDDLRepositories& pddl_repositories) const
{
    return pddl_repositories.get_or_create_literal(is_negated(), m_atom->lift(terms, pddl_repositories));
}

template class GroundLiteralImpl<Static>;
template class GroundLiteralImpl<Fluent>;
template class GroundLiteralImpl<Derived>;

template<PredicateTag P>
std::tuple<VariableList, LiteralList<P>, LiteralList<Static>> lift(const GroundLiteralList<P>& ground_literals, const Domain& domain, PDDLRepositories& pddl_repositories)
{
    VariableList variables;
    LiteralList<P> literals;
    LiteralList<Static> static_literals;
    std::map<Object, Variable> to_variable;

    for (const auto& ground_literal : ground_literals)
    {
        TermList terms;
        for (const auto& object : ground_literal->get_atom()->get_objects())
        {
            if (!to_variable.contains(object))
            {
                const auto parameter_index = to_variable.size();
                const auto variable_name = "x" + std::to_string(parameter_index);
                const auto variable = pddl_repositories.get_or_create_variable(variable_name, parameter_index);
                variables.emplace_back(variable);
                to_variable.emplace(object, variable);
            }
            const auto variable = to_variable.at(object);
            const auto term = pddl_repositories.get_or_create_term(variable);
            terms.emplace_back(term);
        }
        literals.emplace_back(ground_literal->lift(terms, pddl_repositories));
    }

    const auto& static_predicates = domain->get_name_to_predicate<Static>();
    if (static_predicates.contains("="))
    {
        const auto& equality_predicate = static_predicates.at("=");
        for (size_t first_index = 0; first_index < variables.size(); ++first_index)
        {
            for (size_t second_index = first_index + 1; second_index < variables.size(); ++second_index)
            {
                const auto equality_first_term = pddl_repositories.get_or_create_term(variables[first_index]);
                const auto equality_second_term = pddl_repositories.get_or_create_term(variables[second_index]);
                const auto equality_atom = pddl_repositories.get_or_create_atom(equality_predicate, { equality_first_term, equality_second_term });
                const auto equality_literal = pddl_repositories.get_or_create_literal(true, equality_atom);
                static_literals.emplace_back(equality_literal);
            }
        }
    }

    return std::make_tuple(variables, literals, static_literals);
}

template std::tuple<VariableList, LiteralList<Static>, LiteralList<Static>> lift(const GroundLiteralList<Static>&, const Domain&, PDDLRepositories&);
template std::tuple<VariableList, LiteralList<Fluent>, LiteralList<Static>> lift(const GroundLiteralList<Fluent>&, const Domain&, PDDLRepositories&);
template std::tuple<VariableList, LiteralList<Derived>, LiteralList<Static>> lift(const GroundLiteralList<Derived>&, const Domain&, PDDLRepositories&);

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<P>& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<Static>& element);
template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<Fluent>& element);
template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<Derived>& element);

template<PredicateTag P>
std::ostream& operator<<(std::ostream& out, GroundLiteral<P> element)
{
    out << *element;
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundLiteral<Static> element);
template std::ostream& operator<<(std::ostream& out, GroundLiteral<Fluent> element);
template std::ostream& operator<<(std::ostream& out, GroundLiteral<Derived> element);
}
