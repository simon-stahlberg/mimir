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

#ifndef MIMIR_SEARCH_GROUNDING_MATCH_TREE_HPP_
#define MIMIR_SEARCH_GROUNDING_MATCH_TREE_HPP_

#include "mimir/common/types_cista.hpp"

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
    using NodeIndex = size_t;
    using ActionIter = size_t;

    enum class NodeType
    {
        FLUENT_SELECTOR,
        DERIVED_SELECTOR,
        GENERATOR,
    };

    /* We encode selector and generator nodes in a single type */
    struct GeneratorOrSelectorNode
    {
        static const size_t MAX_VALUE = (size_t) -1;

        size_t first_data;
        size_t second_data;
        size_t third_data;
        size_t fourth_data;
        NodeType type;

        /* Common */

        NodeType get_node_type() const noexcept { return type; }

        /* Selector specific accessors */

        /// @brief Partial instantiation of a selector node.
        /// Initialization of true_succ, false_succ, and dont_care are postponed
        explicit GeneratorOrSelectorNode(size_t ground_atom_index, NodeType type) :
            first_data(ground_atom_index),
            second_data(MAX_VALUE),
            third_data(MAX_VALUE),
            fourth_data(MAX_VALUE),
            type(type)
        {
            assert(type == NodeType::FLUENT_SELECTOR || type == NodeType::DERIVED_SELECTOR);
        }

        bool is_selector_node() const noexcept { return first_data != MAX_VALUE; }

        size_t get_ground_atom_index() const noexcept { return first_data; }

        bool has_true_succ() const noexcept { return second_data != MAX_VALUE; }
        NodeIndex get_true_succ() const noexcept { return second_data; }
        void set_true_succ(NodeIndex true_succ) noexcept { second_data = true_succ; }

        bool has_false_succ() const noexcept { return third_data != MAX_VALUE; }
        NodeIndex get_false_succ() const noexcept { return third_data; }
        void set_false_succ(NodeIndex false_succ) noexcept { third_data = false_succ; }

        bool has_dontcare_succ() const noexcept { return fourth_data != MAX_VALUE; }
        NodeIndex get_dontcare_succ() const noexcept { return fourth_data; }
        void set_dontcare_succ(NodeIndex dontcare_succ) noexcept { fourth_data = dontcare_succ; }

        /* Generator specific accessors */

        /// @brief Complete instantiation of a generator node.
        /// The fourth data member is unused.
        GeneratorOrSelectorNode(size_t begin, size_t end, NodeType type) :
            first_data(MAX_VALUE),
            second_data(begin),
            third_data(end),
            fourth_data(MAX_VALUE),
            type(type)
        {
            assert(type == NodeType::GENERATOR);
        }

        bool is_generator_node() const noexcept { return first_data == MAX_VALUE; }

        ActionIter get_begin() const noexcept { return second_data; }

        ActionIter get_end() const noexcept { return third_data; }
    };

    std::vector<GeneratorOrSelectorNode> m_nodes;
    std::vector<T> m_elements;

    NodeIndex build_recursively(const size_t order_pos,
                                const std::vector<T>& elements,
                                const std::vector<size_t>& fluent_ground_atoms_order,
                                const std::vector<size_t>& derived_ground_atoms_order);

    void get_applicable_elements_recursively(size_t node_index,
                                             const FlatBitset& fluent_ground_atoms,
                                             const FlatBitset& derived_ground_atoms,
                                             std::vector<T>& out_applicable_elements);

public:
    MatchTree();
    MatchTree(const std::vector<T>& elements, const std::vector<size_t>& fluent_ground_atoms_order, const std::vector<size_t>& derived_ground_atoms_order);

    void get_applicable_elements(const FlatBitset& fluent_ground_atoms, const FlatBitset& derived_ground_atoms, std::vector<T>& out_applicable_elements);

    size_t get_num_nodes() const;

    void print() const;
};

template<typename T>
MatchTree<T>::NodeIndex MatchTree<T>::MatchTree::build_recursively(const size_t order_pos,
                                                                   const std::vector<T>& elements,
                                                                   const std::vector<size_t>& fluent_ground_atoms_order,
                                                                   const std::vector<size_t>& derived_ground_atoms_order)
{
    const auto num_fluent_atoms = fluent_ground_atoms_order.size();
    const auto num_derived_atoms = derived_ground_atoms_order.size();
    const auto end_pos = num_fluent_atoms + num_derived_atoms;

    // 1. Base cases:

    // 1.1. There are no more atoms to test or
    // 1.2. there are no elements.
    if ((order_pos == end_pos) || (elements.empty()))
    {
        const auto node_index = MatchTree::NodeIndex { m_nodes.size() };
        m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(m_elements.size(), m_elements.size() + elements.size(), NodeType::GENERATOR));
        m_elements.insert(m_elements.end(), elements.begin(), elements.end());
        return node_index;
    }

    // 2. Conquer:

    bool is_fluent = (order_pos < num_fluent_atoms);
    const auto atom_index = (is_fluent) ? fluent_ground_atoms_order.at(order_pos) : derived_ground_atoms_order.at(order_pos - num_fluent_atoms);
    // Partition elements into positive, negative and dontcare depending on how atom_index occurs in precondition
    auto positive_elements = std::vector<T> {};
    auto negative_elements = std::vector<T> {};
    auto dontcare_elements = std::vector<T> {};
    for (const auto& element : elements)
    {
        const bool positive_condition = (is_fluent) ? element->get_strips_precondition().template get_positive_precondition<Fluent>().get(atom_index) :
                                                      element->get_strips_precondition().template get_positive_precondition<Derived>().get(atom_index);
        const bool negative_condition = (is_fluent) ? element->get_strips_precondition().template get_negative_precondition<Fluent>().get(atom_index) :
                                                      element->get_strips_precondition().template get_negative_precondition<Derived>().get(atom_index);

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
        const auto node_index = MatchTree::NodeIndex { m_nodes.size() };

        m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(atom_index, (is_fluent) ? NodeType::FLUENT_SELECTOR : NodeType::DERIVED_SELECTOR));

        const auto true_succ = (!positive_elements.empty()) ?
                                   build_recursively(order_pos + 1, positive_elements, fluent_ground_atoms_order, derived_ground_atoms_order) :
                                   MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
        const auto false_succ = (!negative_elements.empty()) ?
                                    build_recursively(order_pos + 1, negative_elements, fluent_ground_atoms_order, derived_ground_atoms_order) :
                                    MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
        const auto dontcare_succ = (!dontcare_elements.empty()) ?
                                       build_recursively(order_pos + 1, dontcare_elements, fluent_ground_atoms_order, derived_ground_atoms_order) :
                                       MatchTree::GeneratorOrSelectorNode::MAX_VALUE;

        // Update node with computed information
        auto& node = m_nodes.at(node_index);
        node.set_true_succ(true_succ);
        node.set_false_succ(false_succ);
        node.set_dontcare_succ(dontcare_succ);

        return node_index;
    }
    else if (dontcare_elements.size() == 0)
    {
        // All elements were detected to be inapplicable, return a leaf node
        return MatchTree::GeneratorOrSelectorNode::MAX_VALUE;
    }
    else
    {
        // All elements are dontcares, skip creating a node.
        assert(elements.size() == dontcare_elements.size());
        return build_recursively(order_pos + 1, dontcare_elements, fluent_ground_atoms_order, derived_ground_atoms_order);
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
    m_nodes.push_back(MatchTree::GeneratorOrSelectorNode(0, 0, NodeType::GENERATOR));
}

template<typename T>
MatchTree<T>::MatchTree(const std::vector<T>& elements,
                        const std::vector<size_t>& fluent_ground_atoms_order,
                        const std::vector<size_t>& derived_ground_atoms_order)
{
    [[maybe_unused]] const auto root_node_index = build_recursively(0, elements, fluent_ground_atoms_order, derived_ground_atoms_order);

    assert(root_node_index == 0);
}

template<typename T>
void MatchTree<T>::get_applicable_elements_recursively(size_t node_index,
                                                       const FlatBitset& fluent_ground_atoms,
                                                       const FlatBitset& derived_ground_atoms,
                                                       std::vector<T>& out_applicable_elements)
{
    auto& node = m_nodes[node_index];

    if (node.is_selector_node())
    {
        if (node.has_dontcare_succ())
        {
            get_applicable_elements_recursively(node.get_dontcare_succ(), fluent_ground_atoms, derived_ground_atoms, out_applicable_elements);
        }

        const bool is_atom_true = (node.get_node_type() == NodeType::FLUENT_SELECTOR) ? fluent_ground_atoms.get(node.get_ground_atom_index()) :
                                                                                        derived_ground_atoms.get(node.get_ground_atom_index());

        if (is_atom_true)
        {
            if (node.has_true_succ())
            {
                get_applicable_elements_recursively(node.get_true_succ(), fluent_ground_atoms, derived_ground_atoms, out_applicable_elements);
            }
        }
        else
        {
            if (node.has_false_succ())
            {
                get_applicable_elements_recursively(node.get_false_succ(), fluent_ground_atoms, derived_ground_atoms, out_applicable_elements);
            }
        }
    }
    else
    {
        out_applicable_elements.insert(out_applicable_elements.end(), m_elements.begin() + node.get_begin(), m_elements.begin() + node.get_end());
    }
}

template<typename T>
void MatchTree<T>::get_applicable_elements(const FlatBitset& fluent_ground_atoms,
                                           const FlatBitset& derived_ground_atoms,
                                           std::vector<T>& out_applicable_elements)
{
    out_applicable_elements.clear();

    assert(!m_nodes.empty());

    get_applicable_elements_recursively(0, fluent_ground_atoms, derived_ground_atoms, out_applicable_elements);
}
}

#endif
