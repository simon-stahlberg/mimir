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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_MATCH_TREE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_MATCH_TREE_HPP_

#include "mimir/search/flat_types.hpp"

#include <cstdint>
#include <vector>

namespace mimir
{

/**
 * Implementation of https://arxiv.org/pdf/1109.6051.pdf section 5
 *
 * The key idea in our implementation is to store all elements in a vector
 * and store iterator offsets in the generator nodes.
 * Since iterator offsets, atom ids, and node ids are all integers,
 * we can merge selector nodes and generator
 * nodes in a single data type consisting of four integers.
 */
template<typename T>
class MatchTree
{
private:
    using NodeID = size_t;
    using ActionIter = size_t;

    /* We encode selector and generator nodes in a single type */
    struct GeneratorOrSelectorNode
    {
        static const size_t MAX_VALUE = (size_t) -1;

        size_t first_data;
        size_t second_data;
        size_t third_data;
        size_t fourth_data;

        /* Selector specific accessors */

        /// @brief Partial instantiation of a selector node.
        /// Initialization of true_succ, false_succ, and dont_care are postponed
        explicit GeneratorOrSelectorNode(size_t ground_atom_id) :
            first_data(ground_atom_id),
            second_data(MAX_VALUE),
            third_data(MAX_VALUE),
            fourth_data(MAX_VALUE)
        {
        }

        [[nodiscard]] bool is_selector_node() const noexcept { return first_data != MAX_VALUE; }

        [[nodiscard]] size_t get_ground_atom_id() const noexcept { return first_data; }

        [[nodiscard]] bool has_true_succ() const noexcept { return second_data != MAX_VALUE; }
        [[nodiscard]] NodeID get_true_succ() const noexcept { return second_data; }
        void set_true_succ(NodeID true_succ) noexcept { second_data = true_succ; }

        [[nodiscard]] bool has_false_succ() const noexcept { return third_data != MAX_VALUE; }
        [[nodiscard]] NodeID get_false_succ() const noexcept { return third_data; }
        void set_false_succ(NodeID false_succ) noexcept { third_data = false_succ; }

        [[nodiscard]] bool has_dontcare_succ() const noexcept { return fourth_data != MAX_VALUE; }
        [[nodiscard]] NodeID get_dontcare_succ() const noexcept { return fourth_data; }
        void set_dontcare_succ(NodeID dontcare_succ) noexcept { fourth_data = dontcare_succ; }

        /* Generator specific accessors */

        /// @brief Complete instantiation of a generator node.
        /// The fourth data member is unused.
        GeneratorOrSelectorNode(size_t begin, size_t end) : first_data(MAX_VALUE), second_data(begin), third_data(end), fourth_data(MAX_VALUE) {}

        [[nodiscard]] bool is_generator_node() const noexcept { return first_data == MAX_VALUE; }

        [[nodiscard]] ActionIter get_begin() const noexcept { return second_data; }

        [[nodiscard]] ActionIter get_end() const noexcept { return third_data; }
    };

    std::vector<GeneratorOrSelectorNode> m_nodes;
    std::vector<T> m_elements;

    NodeID build_recursively(const size_t order_pos, const std::vector<T>& elements, const std::vector<size_t>& ground_atoms_order);

    void get_applicable_elements_recursively(size_t node_id, const FlatBitset state, std::vector<T>& out_applicable_elements);

public:
    MatchTree();
    MatchTree(const std::vector<T>& elements, const std::vector<size_t>& ground_atoms_order);

    void get_applicable_elements(const FlatBitset state, std::vector<T>& out_applicable_elements);

    [[nodiscard]] size_t get_num_nodes() const;

    void print() const;
};

template<typename T>
MatchTree<T>::NodeID
MatchTree<T>::MatchTree::build_recursively(const size_t order_pos, const std::vector<T>& elements, const std::vector<size_t>& ground_atoms_order)
{
    const auto atom_id = ground_atoms_order[order_pos];
    // 1. Base cases:

    // 1.1. There are no more atoms to test or
    // 1.2. there are no elements.
    if ((order_pos == ground_atoms_order.size()) || (elements.empty()))
    {
        const auto node_id = MatchTree::NodeID { m_nodes.size() };
        m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(m_elements.size(), m_elements.size() + elements.size()));
        m_elements.insert(m_elements.end(), elements.begin(), elements.end());
        return node_id;
    }

    // 2. Conquer:

    // Partition elements into positive, negative and dontcare depending on how atom_id occurs in precondition
    auto positive_elements = std::vector<T> {};
    auto negative_elements = std::vector<T> {};
    auto dontcare_elements = std::vector<T> {};
    for (const auto& element : elements)
    {
        const bool positive_condition = DenseStripsActionPrecondition(element.get_strips_precondition()).get_positive_fluent_precondition().get(atom_id);
        const bool negative_condition = DenseStripsActionPrecondition(element.get_strips_precondition()).get_negative_fluent_precondition().get(atom_id);

        if (positive_condition && negative_condition)
        {
            // Action is inapplicable
        }
        else
        {
            if (negative_condition)
            {
                negative_elements.push_back(element);
            }
            else if (positive_condition)
            {
                positive_elements.push_back(element);
            }
            else
            {
                dontcare_elements.push_back(element);
            }
        }
    }

    // 2. Inductive cases:
    const bool must_split = (positive_elements.size() != 0) || (negative_elements.size() != 0);
    if (must_split)
    {
        // Top-down creation of nodes to ensure in order evaluation, update information after recursion.
        const auto node_id = MatchTree::NodeID { m_nodes.size() };

        m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(atom_id));

        const auto true_succ = (!positive_elements.empty()) ? build_recursively(order_pos + 1, positive_elements, ground_atoms_order) :
                                                              MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
        const auto false_succ = (!negative_elements.empty()) ? build_recursively(order_pos + 1, negative_elements, ground_atoms_order) :
                                                               MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
        const auto dontcare_succ = (!dontcare_elements.empty()) ? build_recursively(order_pos + 1, dontcare_elements, ground_atoms_order) :
                                                                  MatchTree::GeneratorOrSelectorNode::MAX_VALUE;

        // Update node with computed information
        auto& node = m_nodes[node_id];
        node.set_true_succ(true_succ);
        node.set_false_succ(false_succ);
        node.set_dontcare_succ(dontcare_succ);

        return node_id;
    }
    else if (dontcare_elements.size() == 0)
    {
        // All elements were detected to be inapplicable, return a leaf node
        return MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
    }
    else
    {
        // All elements are dontcares, skip creating a node.
        return build_recursively(order_pos + 1, dontcare_elements, ground_atoms_order);
    }
}

template<typename T>
size_t MatchTree<T>::get_num_nodes() const
{
    return m_nodes.size();
}

template<typename T>
void MatchTree<T>::print() const
{
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        const auto& node = m_nodes[i];
    }
}

template<typename T>
MatchTree<T>::MatchTree()
{
    m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(0, 0));
}

template<typename T>
MatchTree<T>::MatchTree(const std::vector<T>& elements, const std::vector<size_t>& ground_atoms_order)
{
    const auto root_node_id = build_recursively(0, elements, ground_atoms_order);

    assert(root_node_id == 0);
    // Prevent unused variable warning when not in debug mode
    (void) root_node_id;
}

template<typename T>
void MatchTree<T>::get_applicable_elements_recursively(size_t node_id, const FlatBitset state, std::vector<T>& out_applicable_elements)
{
    auto& node = m_nodes[node_id];

    if (node.is_selector_node())
    {
        if (node.has_dontcare_succ())
        {
            get_applicable_elements_recursively(node.get_dontcare_succ(), state, out_applicable_elements);
        }

        if (state.get(node.get_ground_atom_id()))
        {
            if (node.has_true_succ())
            {
                get_applicable_elements_recursively(node.get_true_succ(), state, out_applicable_elements);
            }
        }
        else
        {
            if (node.has_false_succ())
            {
                get_applicable_elements_recursively(node.get_false_succ(), state, out_applicable_elements);
            }
        }
    }
    else
    {
        out_applicable_elements.insert(out_applicable_elements.end(), m_elements.begin() + node.get_begin(), m_elements.begin() + node.get_end());
    }
}

template<typename T>
void MatchTree<T>::get_applicable_elements(const FlatBitset state, std::vector<T>& out_applicable_elements)
{
    out_applicable_elements.clear();

    assert(!m_nodes.empty());

    get_applicable_elements_recursively(0, state, out_applicable_elements);
}
}

#endif
