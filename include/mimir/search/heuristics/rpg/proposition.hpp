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

#ifndef MIMIR_SEARCH_HEURISTICS_RPG_PROPOSITION_HPP_
#define MIMIR_SEARCH_HEURISTICS_RPG_PROPOSITION_HPP_

#include "mimir/common/types.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/heuristics/rpg/structures.hpp"

namespace mimir::search::rpg
{
class Proposition
{
public:
    Proposition(Index index, HanaContainer<IndexList, Action, Axiom> is_precondition_of, bool is_goal) :
        m_index(index),
        m_is_precondition_of(std::move(is_precondition_of)),
        m_is_goal(is_goal)
    {
    }

    Index get_index() const { return m_index; }
    template<IsStructure S>
    const IndexList& is_precondition_of() const
    {
        return boost::hana::at_key(m_is_precondition_of, boost::hana::type<S> {});
    }
    const HanaContainer<IndexList, Action, Axiom>& is_precondition_of() const { return m_is_precondition_of; }
    bool is_goal() const { return m_is_goal; }

private:
    Index m_index;
    HanaContainer<IndexList, Action, Axiom> m_is_precondition_of;
    bool m_is_goal;
};

using PropositionList = std::vector<Proposition>;

using PropositionOffsets = HanaContainer<HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag>;

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
inline auto& get(PropositionOffsets& offsets)
{
    return boost::hana::at_key(boost::hana::at_key(offsets, boost::hana::type<R> {}), boost::hana::type<P> {});
}

using AtomIndicesContainer = HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag>;

template<formalism::IsFluentOrDerivedTag P>
inline auto& get(AtomIndicesContainer& container)
{
    return boost::hana::at_key(container, boost::hana::type<P> {});
}
}

#endif
