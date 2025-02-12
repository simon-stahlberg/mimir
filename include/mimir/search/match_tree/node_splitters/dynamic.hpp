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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_DYNAMIC_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_DYNAMIC_HPP_

#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class DynamicNodeSplitter : public INodeSplitter<Element>
{
private:
    const PDDLRepositories& m_pddl_repositories;

public:
    explicit DynamicNodeSplitter(const PDDLRepositories& pddl_repositories);

    std::pair<InverseNode<Element>, PlaceholderNodeList<Element>> compute_best_split(const PlaceholderNode<Element>& node) override;

    InverseNode<Element> translate_to_imperfect_generator_node(const PlaceholderNode<Element>& node) const override;
};

}

#endif
