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
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_score_functions/min_depth.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"
// #include "mimir/search/match_tree/node_splitters/frequency.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/node_splitters/static.hpp"

#include <queue>
#include <vector>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
struct RefineInverseNodeVisitor : public IInverseNodeVisitor<Element>
{
    const InverseNode<Element>& m_parent;
    const NodeSplitter<Element>& m_node_splitter;
    std::vector<InverseNode<Element>>& m_children;

    RefineInverseNodeVisitor(const InverseNode<Element>& parent, const NodeSplitter<Element>& node_splitter, std::vector<InverseNode<Element>>& children) :
        m_parent(parent),
        m_node_splitter(node_splitter),
        m_children(children)
    {
    }

    template<DynamicPredicateTag P>
    void refine_atom(const InverseAtomSelectorNode<Element, P>& atom)
    {
        m_children.push_back(m_node_splitter->compute_node(atom.get_true_elements(), m_parent));
        m_children.push_back(m_node_splitter->compute_node(atom.get_false_elements(), m_parent));
        m_children.push_back(m_node_splitter->compute_node(atom.get_dontcare_elements(), m_parent));
    }

    void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) { refine_atom(atom); }
    void accept(const InverseAtomSelectorNode<Element, Derived>& atom) { refine_atom(atom); }
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint)
    {
        m_children.push_back(m_node_splitter->compute_node(constraint.get_true_elements(), m_parent));
        m_children.push_back(m_node_splitter->compute_node(constraint.get_dontcare_elements(), m_parent));
    }
    void accept(const InverseElementGeneratorNode<Element>& generator)
    {
        // There is nothing to refine.
    }
};

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

    InverseNode<Element> build_root(const NodeSplitter<Element>& node_splitter)
    {
        auto root_span = std::span<const Element*>(m_elements.begin(), m_elements.end());
        auto root_parent = InverseNode<Element> { nullptr };
        auto root_node = node_splitter->compute_node(root_span, root_parent);

        return root_node;
    }

    std::vector<InverseNode<Element>> refine_leaf(const NodeSplitter<Element>& node_splitter, const InverseNode<Element>& element)
    {
        std::cout << "REFINE LEAF" << std::endl;

        auto children = std::vector<InverseNode<Element>> {};
        auto visitor = RefineInverseNodeVisitor(element, node_splitter, children);
        element->visit(visitor);

        return children;
    }

    void build_iteratively(const NodeScoreFunction<Element>& node_score_function, const NodeSplitter<Element>& node_splitter)
    {
        struct QueueEntry
        {
            double score;
            InverseNode<Element> node;
        };

        struct QueueEntryComparator
        {
            bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const { return lhs.score > rhs.score; }
        };

        auto queue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryComparator> {};
        auto root = build_root(node_splitter);
        auto score = node_score_function->compute_score(root);
        queue.emplace(score, root);

        std::cout << "Instantiated ROOT! " << queue.size() << std::endl;

        while (!queue.empty())
        {
            const auto entry = queue.top();
            queue.pop();

            const auto score = entry.score;
            const auto node = entry.node;

            auto children = refine_leaf(node_splitter, node);

            // queue.push_range(children.begin(), children.end()),
        }
    }

public:
    MatchTree() : m_elements(), m_root(build_empty_generator_root()) {}

    MatchTree(std::vector<const Element*> elements,  //
              const NodeScoreFunction<Element>& node_score_function,
              const NodeSplitter<Element>& node_splitter) :
        m_elements(std::move(elements)),
        m_root(build_empty_generator_root())
    {
        if (!m_elements.empty())
        {
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
};

}

#endif
