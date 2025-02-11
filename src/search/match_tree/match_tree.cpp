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

#include "mimir/search/match_tree/match_tree.hpp"

#include "mimir/common/filesystem.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_score_functions/min_depth.hpp"
#include "mimir/search/match_tree/node_splitters/static.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"

#include <optional>
#include <queue>
#include <vector>

namespace mimir::match_tree
{

/* MatchTree */
template<HasConjunctiveCondition Element>
void MatchTree<Element>::build_iteratively(const NodeScoreFunction<Element>& node_score_function, const NodeSplitter<Element>& node_splitter)
{
    struct QueueEntry
    {
        double score;
        PlaceholderNode<Element> node;
    };

    struct QueueEntryComparator
    {
        bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const { return lhs.score > rhs.score; }
    };

    auto queue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryComparator> {};
    auto root_placeholder = create_root_placeholder_node(std::span<const Element*>(m_elements.begin(), m_elements.end()));
    auto score = node_score_function->compute_score(root_placeholder);
    queue.emplace(score, std::move(root_placeholder));
    ++m_num_nodes;

    auto inverse_generator_leafs = InverseNodeList<Element> {};
    auto inverse_root = InverseNode<Element> { nullptr };

    while (!queue.empty())
    {
        auto node = std::move(const_cast<QueueEntry&>(queue.top()).node);
        queue.pop();

        auto [root, children] = node_splitter->compute_best_split(node);

        ++m_num_nodes;

        for (auto& child : children)
        {
            queue.emplace(node_score_function->compute_score(child), std::move(child));
            ++m_num_nodes;
        }
        if (root)
        {
            inverse_root = std::move(root);
        }
        if (m_num_nodes >= m_max_num_nodes)
        {
            /* Mark the tree as imperfect and finish translation of placeholders to generator nodes. */
            m_is_imperfect = true;
            while (!queue.empty())
            {
                auto node = std::move(const_cast<QueueEntry&>(queue.top()).node);
                queue.pop();
                node_splitter->translate_to_generator_node(node);
            }
            break;
        }
    }

    if (m_enable_dump_dot_file)
    {
        auto ss = std::stringstream {};
        ss << std::make_tuple(std::cref(inverse_root), DotPrinterTag {}) << std::endl;
        write_to_file(m_output_dot_file, ss.str());
    }

    m_root = parse_inverse_tree_iteratively(inverse_root);
}

template<HasConjunctiveCondition Element>
MatchTree<Element>::MatchTree() :
    m_elements(),
    m_root(create_root_generator_node(std::span<const Element*>(m_elements.begin(), m_elements.end()))),
    m_max_num_nodes(std::numeric_limits<size_t>::max()),
    m_enable_dump_dot_file(false),
    m_output_dot_file("match_tree.dot"),
    m_num_nodes(1),
    m_is_imperfect(false)
{
}

template<HasConjunctiveCondition Element>
MatchTree<Element>::MatchTree(std::vector<const Element*> elements,  //
                              const NodeScoreFunction<Element>& node_score_function,
                              const NodeSplitter<Element>& node_splitter,
                              size_t max_num_nodes,
                              bool enable_dump_dot_file,
                              fs::path output_dot_file) :
    m_elements(std::move(elements)),
    m_root(create_root_generator_node(std::span<const Element*>(m_elements.begin(), m_elements.end()))),
    m_max_num_nodes(max_num_nodes),
    m_enable_dump_dot_file(enable_dump_dot_file),
    m_output_dot_file(output_dot_file),
    m_num_nodes(1),
    m_is_imperfect(false)
{
    if (!m_elements.empty())
    {
        m_num_nodes = 0;
        build_iteratively(node_score_function, node_splitter);
    }
}

template<HasConjunctiveCondition Element>
void MatchTree<Element>::generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements)
{
    m_evaluate_stack.clear();
    out_applicable_elements.clear();

    m_evaluate_stack.push_back(m_root.get());

    while (!m_evaluate_stack.empty())
    {
        const auto node = m_evaluate_stack.back();

        m_evaluate_stack.pop_back();

        node->generate_applicable_actions(state, m_evaluate_stack, out_applicable_elements);
    }
}

template<HasConjunctiveCondition Element>
size_t MatchTree<Element>::get_num_nodes() const
{
    return m_num_nodes;
}

template<HasConjunctiveCondition Element>
bool MatchTree<Element>::is_imperfect() const
{
    return m_is_imperfect;
}

template<HasConjunctiveCondition Element>
std::unique_ptr<MatchTree<Element>>
MatchTree<Element>::create(const PDDLRepositories& pddl_repositories, std::vector<const Element*> elements, const Options& options)
{
    if (elements.empty())
    {
        return std::unique_ptr<MatchTree<Element>>(new MatchTree<Element>());
    }

    auto node_splitter = NodeSplitter<Element> { nullptr };
    switch (options.split_strategy)
    {
        case SplitStrategyEnum::STATIC:
        {
            node_splitter = std::make_unique<StaticNodeSplitter<Element>>(pddl_repositories, options.split_metric, elements);
            break;
        }
        case SplitStrategyEnum::HYBRID:
        {
            throw std::runtime_error("Not implemented.");
        }
        case SplitStrategyEnum::DYNAMIC:
        {
            throw std::runtime_error("Not implemented.");
        }
        default:
        {
            throw std::logic_error("MatchTree::create: Undefined SplitStrategyEnum type.");
        }
    }

    auto node_score_function = NodeScoreFunction<Element> { nullptr };
    switch (options.node_score_strategy)
    {
        case NodeScoreStrategyEnum::MIN_DEPTH:
        {
            node_score_function = std::make_unique<MinDepthNodeScoreFunction<Element>>();
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

    return std::unique_ptr<MatchTree<Element>>(new MatchTree<Element>(std::move(elements),
                                                                      node_score_function,
                                                                      node_splitter,
                                                                      options.max_num_nodes,
                                                                      options.enable_dump_dot_file,
                                                                      options.output_dot_file));
}

template class MatchTree<GroundActionImpl>;
template class MatchTree<GroundAxiomImpl>;
}
