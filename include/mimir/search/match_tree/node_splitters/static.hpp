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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_STATIC_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_STATIC_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/base.hpp"

#include <map>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class StaticNodeSplitter : public NodeSplitterBase<StaticNodeSplitter<Element>, Element>
{
private:
    SplitList m_splits;

    SplitList compute_statically_best_splits(const std::vector<const Element*>& elements);

    /* Implement NodeSplitterBase interface */

    std::pair<InverseNode<Element>, PlaceholderNodeList<Element>> split_node_impl(PlaceholderNodeList<Element>& ref_leafs);

    friend class NodeSplitterBase<StaticNodeSplitter<Element>, Element>;

public:
    StaticNodeSplitter(const PDDLRepositories& pddl_repositories, const Options& options, const std::vector<const Element*>& elements);
};

}

#endif
