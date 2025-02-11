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

#include "mimir/search/match_tree/nodes/generator.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
ElementGeneratorNode<Element>::ElementGeneratorNode(std::span<const Element*> elements) : m_elements(elements)
{
    // m_elements may be empty, e.g., if there are no ground axioms.
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode<Element>::generate_applicable_actions(const DenseState& state,
                                                                std::vector<const INode<Element>*>&,
                                                                std::vector<const Element*>& ref_applicable_elements) const
{
    ref_applicable_elements.insert(ref_applicable_elements.end(), m_elements.begin(), m_elements.end());
}

template<HasConjunctiveCondition Element>
std::span<const Element*> ElementGeneratorNode<Element>::get_elements() const
{
    return m_elements;
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode<Element>::visit(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template class ElementGeneratorNode<GroundActionImpl>;
template class ElementGeneratorNode<GroundAxiomImpl>;

}
