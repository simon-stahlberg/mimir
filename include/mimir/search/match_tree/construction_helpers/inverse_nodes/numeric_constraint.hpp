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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_NUMERIC_CONSTRAINT_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_NUMERIC_CONSTRAINT_HPP_

#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class InverseNumericConstraintSelectorNode : public IInverseNode<Element>
{
private:
    // Meta data
    GroundNumericConstraint m_constraint;

    // Candidates for further refinement.
    // Note: if two constraints are exactly the opposite, we add true/false successors.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_dontcare_elements;

public:
    InverseNumericConstraintSelectorNode(InverseNode<Element> parent,
                                         SplitList useless_splits,
                                         size_t root_distance,
                                         GroundNumericConstraint constraint,
                                         std::span<const Element*> true_elements,
                                         std::span<const Element*> dontcare_elements) :
        IInverseNode<Element>(parent, std::move(useless_splits), root_distance),
        m_constraint(constraint),
        m_true_elements(true_elements),
        m_dontcare_elements(dontcare_elements)
    {
    }

    void visit(IInverseNodeVisitor<Element>& visitor) const override { visitor.accept(*this); }

    GroundNumericConstraint get_constraint() const { return m_constraint; }
    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }
};
}

#endif
