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

#ifndef MIMIR_FORMALISM_GROUND_ATOM_HPP_
#define MIMIR_FORMALISM_GROUND_ATOM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

template<PredicateTag P>
class GroundAtomImpl
{
private:
    Index m_index;
    Predicate<P> m_predicate;
    ObjectList m_objects;

    // Below: add additional members if needed and initialize them in the constructor

    GroundAtomImpl(Index index, Predicate<P> predicate, ObjectList objects);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using Category = P;

    // moveable but not copyable
    GroundAtomImpl(const GroundAtomImpl& other) = delete;
    GroundAtomImpl& operator=(const GroundAtomImpl& other) = delete;
    GroundAtomImpl(GroundAtomImpl&& other) = default;
    GroundAtomImpl& operator=(GroundAtomImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
    const ObjectList& get_objects() const;
    size_t get_arity() const;
    Atom<P> lift(const TermList& terms, PDDLRepositories& pddl_repositories) const;
};

template<PredicateTag P>
extern std::pair<VariableList, AtomList<P>> lift(const GroundAtomList<P>& ground_atoms, PDDLRepositories& pddl_repositories);

template<PredicateTag P>
extern std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<P>& element);

template<PredicateTag P>
extern std::ostream& operator<<(std::ostream& out, GroundAtom<P> element);

}

#endif
