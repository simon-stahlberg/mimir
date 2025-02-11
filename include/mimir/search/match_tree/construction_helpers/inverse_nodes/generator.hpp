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
    // Final elements
    std::span<const Element*> m_elements;

public:
    InverseElementGeneratorNode(const IInverseNode<Element>* parent, size_t root_distance, std::span<const Element*> elements);

    InverseElementGeneratorNode(const InverseElementGeneratorNode& other) = delete;
    InverseElementGeneratorNode& operator=(const InverseElementGeneratorNode& other) = delete;
    InverseElementGeneratorNode(InverseElementGeneratorNode&& other) = delete;
    InverseElementGeneratorNode& operator=(InverseElementGeneratorNode&& other) = delete;

    void visit(IInverseNodeVisitor<Element>& visitor) const override;

    std::span<const Element*> get_elements() const;
};
}

#endif
