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
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/utils.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/queue_entry_scoring_functions/min_depth.hpp"
// #include "mimir/search/match_tree/split_scoring_functions/frequency.hpp"
#include "mimir/search/match_tree/split_scoring_functions/static_frequency.hpp"

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
    Node<Element> m_root;                    ///< the root node.

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    /// @brief Build the empty root node.
    /// @return
    Node<Element> build_empty_generator_root()
    {
        return std::make_unique<ElementGeneratorNode<Element>>(std::span<const Element*>(m_elements.begin(), m_elements.end()));
    }

    InverseNode<Element> build_root(const SplitScoringFunction<Element>& split_scoring_function)
    {
        auto root_span = std::span<const Element*>(m_elements.begin(), m_elements.end());
        auto root_split_result = split_scoring_function->compute_best_split(root_span);
        auto parent = InverseNode<Element> { nullptr };

        if (!root_split_result.has_value())
        {
            return std::make_shared<InverseElementGeneratorNode<Element>>(parent, 0, root_span);  // This branch is very unlikely.
        }

        return create_node_from_split(root_split_result.value(), root_span, parent);
    }

    std::vector<InverseNode<Element>> refine_leaf(const QueueEntryScoringFunction<Element>& queue_scoring_function,
                                                  const SplitScoringFunction<Element>& split_scoring_function,
                                                  const InverseNode<Element>& element)
    {
    }

    void build_iteratively(const QueueEntryScoringFunction<Element>& queue_scoring_function, const SplitScoringFunction<Element>& split_scoring_function)
    {
        struct QueueEntryComparator
        {
            bool operator()(const InverseNode<Element>& lhs, const InverseNode<Element>& rhs) const { return lhs->get_queue_score() > rhs->get_queue_score(); }
        };

        auto queue = std::priority_queue<InverseNode<Element>, std::vector<InverseNode<Element>>, QueueEntryComparator> {};
        auto root = build_root(split_scoring_function);
        queue.emplace(root);

        std::cout << "Instantiated ROOT! " << queue.size() << std::endl;

        while (!queue.empty())
        {
            const auto node = queue.top();
            queue.pop();

            auto children = refine_leaf(queue_scoring_function, split_scoring_function, node);

            // queue.push_range(children.begin(), children.end()),
        }
    }

public:
    MatchTree() : m_elements(), m_root(build_empty_generator_root()) {}

    MatchTree(std::vector<const Element*> elements,  //
              const QueueEntryScoringFunction<Element>& queue_scoring_function,
              const SplitScoringFunction<Element>& split_scoring_function) :
        m_elements(std::move(elements)),
        m_root(build_empty_generator_root())
    {
        if (!m_elements.empty())
        {
            build_iteratively(queue_scoring_function, split_scoring_function);
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
};

}

#endif
