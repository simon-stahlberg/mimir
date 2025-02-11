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

#ifndef MIMIR_SEARCH_MATCH_TREE_INSTANTIATION_HPP_
#define MIMIR_SEARCH_MATCH_TREE_INSTANTIATION_HPP_

#include "mimir/search/match_tree/match_tree.hpp"

namespace mimir::match_tree
{
struct Options
{
    std::optional<fs::path> output_dot_file = std::nullopt;
    size_t max_num_nodes = std::numeric_limits<size_t>::max();
    SplitMetricEnum split_metric = SplitMetricEnum::GINI;
    SplitStrategyEnum split_strategy = SplitStrategyEnum::STATIC;
    NodeScoreStrategyEnum node_score_strategy = NodeScoreStrategyEnum::MIN_DEPTH;
};

template<HasConjunctiveCondition Element>
static std::unique_ptr<MatchTree<Element>> create(std::vector<const Element*> elements, const Options& options = Options())
{
    if (elements.empty())
    {
        return std::make_unique<MatchTree<Element>>();
    }

    auto node_splitter = std::unique_ptr<NodeSplitter<Element>> { nullptr };
    switch (options.split_strategy)
    {
        case SplitStrategyEnum::DYNAMIC:
        {
            throw std::runtime_error("Not implemented.");
        }
        case SplitStrategyEnum::HYBRID:
        {
            throw std::runtime_error("Not implemented.");
        }
        case SplitStrategyEnum::STATIC:
        {
            // TODO
            break;
        }
        default:
        {
            throw std::logic_error("MatchTree::create: Undefined SplitStrategyEnum type.");
        }
    }

    auto node_score_function = std::unique_ptr<NodeScoreFunction<Element>> { nullptr };
    switch (options.node_score_strategy)
    {
        case NodeScoreStrategyEnum::MIN_DEPTH:
        {
            break;
        }
        case NodeScoreStrategyEnum::MAX_DEPTH:
        {
            break;
        }
        case NodeScoreStrategyEnum::MIN_BREADTH:
        {
            break;
        }
        case NodeScoreStrategyEnum::MAX_BREADTH:
        {
            break;
        }
        default:
        {
            throw std::logic_error("MatchTree::create: Undefined NodeScoreStrategyEnum type.");
        }
    }
}
}

#endif
