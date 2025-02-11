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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_GENERATOR_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_GENERATOR_HPP_

#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class ElementGeneratorNode : public INode<Element>
{
private:
    std::span<const Element*> m_elements;

public:
    explicit ElementGeneratorNode(std::span<const Element*> elements);

    ElementGeneratorNode(const ElementGeneratorNode& other) = delete;
    ElementGeneratorNode& operator=(const ElementGeneratorNode& other) = delete;
    ElementGeneratorNode(ElementGeneratorNode&& other) = delete;
    ElementGeneratorNode& operator=(ElementGeneratorNode&& other) = delete;

    void generate_applicable_actions(const DenseState& state,
                                     std::vector<const INode<Element>*>&,
                                     std::vector<const Element*>& ref_applicable_elements) const override;

    std::span<const Element*> get_elements() const;

    void visit(INodeVisitor<Element>& visitor) const override;
};
}

#endif
