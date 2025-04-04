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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
PlaceholderNodeImpl<E>::PlaceholderNodeImpl(const IInverseNode<E>* parent, InverseNode<E>* parents_child, std::span<const E*> elements) :
    m_parent(parent),
    m_parents_child(parents_child),
    m_elements(elements)
{
}

template<formalism::HasConjunctiveCondition E>
const IInverseNode<E>* PlaceholderNodeImpl<E>::get_parent() const
{
    return m_parent;
}

template<formalism::HasConjunctiveCondition E>
InverseNode<E>& PlaceholderNodeImpl<E>::get_parents_child() const
{
    assert(m_parents_child);
    return *m_parents_child;
}

template<formalism::HasConjunctiveCondition E>
std::span<const E*> PlaceholderNodeImpl<E>::get_elements() const
{
    return m_elements;
}

template class PlaceholderNodeImpl<GroundActionImpl>;
template class PlaceholderNodeImpl<GroundAxiomImpl>;
}
