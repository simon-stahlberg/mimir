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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
InverseElementGeneratorNode<Element>::InverseElementGeneratorNode(const IInverseNode<Element>* parent,
                                                                  size_t root_distance,
                                                                  std::span<const Element*> elements) :
    IInverseNode<Element>(parent, SplitList {}, root_distance),
    m_elements(elements)
{
    assert(!m_elements.empty());
}

template<HasConjunctiveCondition Element>
void InverseElementGeneratorNode<Element>::visit(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element>
std::span<const Element*> InverseElementGeneratorNode<Element>::get_elements() const
{
    return m_elements;
}

template class InverseElementGeneratorNode<GroundActionImpl>;
template class InverseElementGeneratorNode<GroundAxiomImpl>;

}
