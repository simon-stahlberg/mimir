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
#include "mimir/search/match_tree/node_splitters/base.hpp"

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E>
class DynamicNodeSplitter : public NodeSplitterBase<DynamicNodeSplitter<E>, E>
{
private:
    /* Implement NodeSplitterBase interface */

    InverseNode<E> fit_impl(std::span<const E*> elements, Statistics& ref_statistics);

    friend class NodeSplitterBase<DynamicNodeSplitter<E>, E>;

public:
    DynamicNodeSplitter(const formalism::Repositories& pddl_repositories, const Options& options, std::span<const E*> elements);
};

}

#endif
