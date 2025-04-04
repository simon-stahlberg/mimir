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

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E>
StaticNodeSplitter<E>::StaticNodeSplitter(const Repositories& pddl_repositories, const Options& options, std::span<const E*> elements) :
    NodeSplitterBase<StaticNodeSplitter<E>, E>(pddl_repositories, options),
    m_split_to_root_distance()
{
    auto splits = this->compute_splits(elements);

    auto split_and_score_list = SplitAndScoreList {};
    for (const auto& split : splits)
    {
        split_and_score_list.push_back(SplitAndScore { split, compute_score(split, options.split_metric) });
    }

    std::sort(split_and_score_list.begin(), split_and_score_list.end(), [](auto&& lhs, auto&& rhs) { return lhs.score > rhs.score; });

    std::cout << "[MatchTree] Static split ordering determined with " << to_string(this->m_options.split_metric) << " score: " << std::endl;
    for (size_t i = 0; i < splits.size(); ++i)
    {
        const auto& split_and_score = split_and_score_list[i];
        std::cout << "    " << i << ". ";
        mimir::operator<<(std::cout, split_and_score.split);
        std::cout << " with score " << split_and_score.score << std::endl;
    }

    // Initialize extra bookkeeping to obtain the queue priority score of a node, i.e., the root_distance.
    for (size_t root_distance = 0; root_distance < splits.size(); ++root_distance)
    {
        const auto& split_and_score = split_and_score_list[root_distance];
        m_split_to_root_distance.emplace(split_and_score.split, root_distance);
    }
}

template<formalism::HasConjunctiveCondition E>
InverseNode<E> StaticNodeSplitter<E>::fit_impl(std::span<const E*> elements, Statistics& ref_statistics)
{
    throw std::runtime_error("Not implemented.");
}

template class StaticNodeSplitter<GroundActionImpl>;
template class StaticNodeSplitter<GroundAxiomImpl>;

}
