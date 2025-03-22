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

#include "mimir/formalism/ground_atom.hpp"

#include "formatter.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/repositories.hpp"

#include <map>

namespace mimir::formalism
{
template<IsStaticOrFluentOrDerivedTag P>
GroundAtomImpl<P>::GroundAtomImpl(Index index, Predicate<P> predicate, ObjectList objects) :
    m_index(index),
    m_predicate(std::move(predicate)),
    m_objects(std::move(objects))
{
}

template<IsStaticOrFluentOrDerivedTag P>
Index GroundAtomImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> GroundAtomImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<IsStaticOrFluentOrDerivedTag P>
const ObjectList& GroundAtomImpl<P>::get_objects() const
{
    return m_objects;
}

template<IsStaticOrFluentOrDerivedTag P>
size_t GroundAtomImpl<P>::get_arity() const
{
    return m_objects.size();
}

template<IsStaticOrFluentOrDerivedTag P>
Atom<P> GroundAtomImpl<P>::lift(const TermList& terms, Repositories& pddl_repositories) const
{
    return pddl_repositories.get_or_create_atom(m_predicate, terms);
}

template class GroundAtomImpl<StaticTag>;
template class GroundAtomImpl<FluentTag>;
template class GroundAtomImpl<DerivedTag>;

template<IsStaticOrFluentOrDerivedTag P>
std::pair<VariableList, AtomList<P>> lift(const GroundAtomList<P>& ground_atoms, Repositories& pddl_repositories)
{
    VariableList variables;
    AtomList<P> atoms;
    std::map<Object, Variable> to_variable;
    for (const auto& ground_atom : ground_atoms)
    {
        TermList terms;
        for (const auto& object : ground_atom->get_objects())
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
        atoms.emplace_back(ground_atom->lift(terms, pddl_repositories));
    }
    return std::make_pair(variables, atoms);
}

template std::pair<VariableList, AtomList<StaticTag>> lift(const GroundAtomList<StaticTag>&, Repositories&);
template std::pair<VariableList, AtomList<FluentTag>> lift(const GroundAtomList<FluentTag>&, Repositories&);
template std::pair<VariableList, AtomList<DerivedTag>> lift(const GroundAtomList<DerivedTag>&, Repositories&);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<P>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<DerivedTag>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, GroundAtom<P> element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, GroundAtom<StaticTag> element);
template std::ostream& operator<<(std::ostream& out, GroundAtom<FluentTag> element);
template std::ostream& operator<<(std::ostream& out, GroundAtom<DerivedTag> element);
}
