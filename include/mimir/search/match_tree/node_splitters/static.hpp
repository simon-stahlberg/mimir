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

#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/base.hpp"

#include <map>
#include <unordered_map>

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
class StaticNodeSplitter : public NodeSplitterBase<StaticNodeSplitter<E>, E>
{
private:
    std::unordered_map<Split, size_t, loki::Hash<Split>, loki::EqualTo<Split>> m_split_to_root_distance;

    /* Implement NodeSplitterBase interface */

    InverseNode<E> fit_impl(std::span<const E*> elements, Statistics& ref_statistics);

    friend class NodeSplitterBase<StaticNodeSplitter<E>, E>;

public:
    StaticNodeSplitter(const formalism::Repositories& pddl_repositories, const Options& options, std::span<const E*> elements);
};

}

#endif
