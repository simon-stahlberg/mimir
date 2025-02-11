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

#ifndef MIMIR_SEARCH_MATCH_TREE_MATCH_TREE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_MATCH_TREE_HPP_

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_score_functions/min_depth.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
// #include "mimir/search/match_tree/node_splitters/frequency.hpp"
#include "mimir/common/filesystem.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/node_splitters/static.hpp"

#include <optional>
#include <queue>
#include <vector>

namespace mimir::match_tree
{

/* MatchTree */
template<HasConjunctiveCondition Element>
class MatchTree
{
private:
    std::vector<const Element*> m_elements;  ///< ATTENTION: must remain persistent. Swapping elements is allowed.
    Node<Element> m_root;                    ///< The root node.
    size_t m_max_num_nodes;                  ///< The maximum number of nodes allowed in the tree. Might result in imperfect match trees.

    bool m_enable_dump_dot_file;
    fs::path m_output_dot_file;

    /* Statistics */
    size_t m_num_nodes;
    bool m_is_imperfect;

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    void build_iteratively(const NodeScoreFunction<Element>& node_score_function, const NodeSplitter<Element>& node_splitter)
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

        auto inverse_generator_leafs = InverseNodeList<Element> {};
        auto inverse_root = InverseNode<Element> { nullptr };

        while (!queue.empty())
        {
            auto node = std::move(const_cast<QueueEntry&>(queue.top()).node);
            queue.pop();

            auto [root, children] = node_splitter->compute_best_split(node);

            ++m_num_nodes;

            if (m_num_nodes >= m_max_num_nodes)
            {
                m_is_imperfect = true;  ///< If we terminate early, we assume imperfection.
                break;
            }

            for (auto& child : children)
            {
                queue.emplace(node_score_function->compute_score(child), std::move(child));
            }
            if (root)
            {
                inverse_root = std::move(root);
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

public:
    // TODO: do we need this default constructor?
    MatchTree() :
        m_elements(),
        m_root(create_root_generator_node(std::span<const Element*>(m_elements.begin(), m_elements.end()))),
        m_max_num_nodes(std::numeric_limits<size_t>::max()),
        m_enable_dump_dot_file(false),
        m_output_dot_file("match_tree.dot"),
        m_num_nodes(1),
        m_is_imperfect(false)
    {
    }

    MatchTree(std::vector<const Element*> elements,  //
              const NodeScoreFunction<Element>& node_score_function,
              const NodeSplitter<Element>& node_splitter,
              size_t max_num_nodes = std::numeric_limits<size_t>::max(),
              bool enable_dump_dot_file = false,
              fs::path output_dot_file = fs::path("match_tree.dot")) :
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

    // Uncopieable and unmoveable to prohibit invalidating spans on m_elements.
    MatchTree(const MatchTree& other) = delete;
    MatchTree& operator=(const MatchTree& other) = delete;
    MatchTree(MatchTree&& other) = delete;
    MatchTree& operator=(MatchTree&& other) = delete;

    void generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements)
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

    size_t get_num_nodes() const { return m_num_nodes; }

    bool is_imperfect() const { return m_is_imperfect; }
};

}

#endif
