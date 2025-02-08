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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_GENERATOR_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_GENERATOR_HPP_

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class InverseElementGeneratorNode : public IInverseNode<Element>
{
private:
    // Meta data
    InverseNode<Element> m_parent;
    double m_queue_score;

    // Final elements
    std::span<const Element*> m_elements;

public:
    InverseElementGeneratorNode(InverseNode<Element> parent, double queue_score, std::span<const Element*> elements) :
        m_parent(parent),
        m_queue_score(queue_score),
        m_elements(elements)
    {
    }

    void visit(IInverseNodeVisitor<Element>& visitor) const override { visitor.accept(*this); }

    const InverseNode<Element>& get_parent() const override { return m_parent; }
    double get_queue_score() const override { return m_queue_score; }

    std::span<const Element*> get_elements() const { return m_elements; }
};
}

#endif
