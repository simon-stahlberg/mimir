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
    size_t m_root_distance;
    InverseNode<Element> m_parent;
    double m_queue_score;
    GroundNumericConstraint m_constraint;

    // Candidates for further refinement.
    // Note: if two constraints are exactly the opposite, we add true/false successors.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_dontcare_elements;

public:
    InverseNumericConstraintSelectorNode(size_t root_distance,
                                         InverseNode<Element> parent,
                                         double queue_score,
                                         GroundNumericConstraint constraint,
                                         std::span<const Element*> true_elements,
                                         std::span<const Element*> dontcare_elements) :
        m_root_distance(root_distance),
        m_parent(parent),
        m_queue_score(queue_score),
        m_constraint(constraint),
        m_true_elements(true_elements),
        m_dontcare_elements(dontcare_elements)
    {
    }

    void visit(IInverseNodeVisitor<Element>& visitor) const override { visitor.accept(*this); }

    size_t get_root_distance() const override { return m_root_distance; }
    const InverseNode<Element>& get_parent() const override { return m_parent; }
    double get_queue_score() const override { return m_queue_score; }

    GroundNumericConstraint get_constraint() const { return m_constraint; }
    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }
};
}

#endif
