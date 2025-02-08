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

#ifndef MIMIR_SEARCH_MATCH_TREE2_HPP_
#define MIMIR_SEARCH_MATCH_TREE2_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"

#include <cstdint>
#include <queue>
#include <vector>

namespace mimir::match_tree
{

/**
 * Base classes
 */

/// @brief `Node` implements the interface of nodes.
template<typename Element>
class INode
{
public:
    ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             std::vector<const INode<Element>*>& ref_applicable_nodes,
                                             std::vector<Element>& ref_applicable_elements) const = 0;
};

template<typename Element>
using Node = std::unique_ptr<INode<Element>>;

/* Customization point 1: NodeScoringFunction*/

template<typename Element>
struct QueueEntry
{
    double score;
    const Node<Element>* node;
};

template<typename Element>
struct QueueEntryComparator
{
    bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const { return lhs.score > rhs.score; }
};

/// @brief `INodeScoringFunction` allows computing a score for a node.
template<typename Element>
class IQueueEntryScoringFunction
{
public:
    virtual ~IQueueEntryScoringFunction() = default;

    virtual double compute_score(const QueueEntry<Element>& entry, size_t distance_to_root) = 0;
};

template<typename Element>
using QueueEntryScoringFunction = std::unique_ptr<IQueueEntryScoringFunction<Element>>;

using Split = std::variant<Literal<Fluent>, Literal<Derived>, NumericConstraint>;

/* Customization point 2: SplitScoringFunction */

/// @brief `ISplitScoringFunction` computes the best split for a given set of elements.
/// @tparam Element
template<typename Element>
class ISplitScoringFunction
{
public:
    virtual ~ISplitScoringFunction() = default;

    virtual Split compute_best_split(std::span<Element> elements) = 0;
};

template<typename Element>
using SplitScoringFunction = std::unique_ptr<ISplitScoringFunction<Element>>;

/**
 * Derived classes
 */

/* Nodes */
template<typename Element, DynamicPredicateTag P>
class LiteralSelectorNode : public INode<Element>
{
private:
    Index m_atom_index;
    bool m_is_negated;

    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    explicit LiteralSelectorNode(GroundLiteral<P> literal) : m_atom_index(literal->get_atom()->get_index()), m_is_negated(literal->is_negated()) {}

    void generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<Element>&) const override
    {
        (state.get_atoms<P>().get(m_atom_index) != m_is_negated) ? ref_applicable_nodes.push_back(m_true_succ) : ref_applicable_nodes.push_back(m_false_succ);
        ref_applicable_nodes.push_back(m_dontcare_succ);
    }
};

template<typename Element>
class NumericConstraintSelectorNode : public INode<Element>
{
private:
    GroundNumericConstraint m_constraint;

    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    void generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<Element>&) const override
    {
        evaluate(m_constraint, state.get_numeric_variables()) ? ref_applicable_nodes.push_back(m_true_succ) : ref_applicable_nodes.push_back(m_false_succ);
        ref_applicable_nodes.push_back(m_dontcare_succ);
    }
};

template<typename Element>
class ElementGeneratorNode : public INode<Element>
{
private:
    std::span<Element> m_elements;

public:
    void generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>&, std::vector<Element>& ref_applicable_elements) const override
    {
        ref_applicable_elements.insert(ref_applicable_elements.end(), m_elements.begin(), m_elements.end());
    }
};

/* MatchTree */
template<typename Element>
class MatchTree
{
private:
    std::vector<Element> m_elements;  ///< ATTENTION: must remain persistent. Swapping elements is allowed.
    Node<Element> m_root;             ///< the root node.

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    void build_iteratively(const QueueEntryScoringFunction& queue_scoring_function, const SplitScoringFunction& split_scoring_function)
    {
        auto queue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryComparator> {};
    }

public:
    MatchTree(std::vector<Element> elements,  //
              const QueueEntryScoringFunction& queue_scoring_function,
              const SplitScoringFunction& split_scoring_function) :
        m_elements(std::move(elements))
    {
        build_iteratively(queue_scoring_function, split_scoring_function);
    }

    void generate_applicable_elements_iteratively(const DenseState& state, std::vector<Element>& out_applicable_elements)
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
