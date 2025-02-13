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

#include "mimir/search/match_tree/node_splitters/static.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/node_splitters/base_impl.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

#include <map>

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
StaticNodeSplitter<Element>::StaticNodeSplitter(const PDDLRepositories& pddl_repositories,
                                                const Options& options,
                                                const std::vector<const Element*>& elements) :
    NodeSplitterBase<StaticNodeSplitter<Element>, Element>(pddl_repositories, options),
    m_cur_split_index(0),
    m_static_splits(this->compute_splits(elements))
{
    std::cout << "[MatchTree] Static split ordering determined with " << to_string(this->m_options.split_metric) << " score: " << std::endl;
    for (size_t i = 0; i < m_static_splits.size(); ++i)
    {
        std::cout << "    " << i << ". " << m_static_splits[i] << std::endl;
    }
}

template<HasConjunctiveCondition Element>
std::pair<InverseNode<Element>, PlaceholderNodeList<Element>> StaticNodeSplitter<Element>::split_node_impl(PlaceholderNodeList<Element>& ref_leafs)
{
    assert(!ref_leafs.empty());

    /*
        for (; m_cur_split_index < m_static_splits.size(); ++m_cur_split_index)
        {
            const auto& cur_split = m_static_splits.at(m_cur_split_index);

            for (auto it = ref_leafs.begin(); it != ref_leafs.end(); ++it)
            {
                auto& leaf = *it;
                if (!m_cached_leaf_precondition_distributions.contains(leaf.get()))
                {
                    m_cached_leaf_precondition_distributions[leaf.get()] = this->compute_splits(leaf->get_elements());
                }
                const auto& leaf_precondition_distribution = m_cached_leaf_precondition_distributions.at(leaf.get());

                if (is_useless_split(leaf_precondition_distribution.))
            }
        }
        */

    auto min_root_distance = std::numeric_limits<size_t>::max();
    auto selected_leaf_it = ref_leafs.begin();
    for (auto it = ref_leafs.begin(); it != ref_leafs.end(); ++it)
    {
        if ((*it)->get_root_distance() < min_root_distance)
        {
            selected_leaf_it = it;
            min_root_distance = (*it)->get_root_distance();
        }
    }

    auto selected_leaf = std::move(*selected_leaf_it);
    ref_leafs.erase(selected_leaf_it);

    auto useless_splits = SplitList {};

    for (size_t i = selected_leaf->get_root_distance(); i < m_static_splits.size(); ++i)
    {
        auto result = create_node_and_placeholder_children(selected_leaf, useless_splits, i, m_static_splits[i]);

        if (result.has_value())
        {
            return std::move(result.value());
        }

        useless_splits.push_back(m_static_splits[i]);
    }

    return create_perfect_generator_node(selected_leaf);
}

template class StaticNodeSplitter<GroundActionImpl>;
template class StaticNodeSplitter<GroundAxiomImpl>;

}
