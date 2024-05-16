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
 * Dominik (2024-04-20): I tried an implementation with std::variant
 * which has similar performance so I switched the to implementation
 * using virtual, which is simpler.
 */
template<typename T>
class MatchTree
{
private:
    using NodeID = size_t;
    using ActionIter = size_t;

    struct SelectorNode
    {
        size_t ground_atom_id;
        NodeID true_succ;
        NodeID false_succ;
        NodeID dontcare_succ;

        explicit SelectorNode(size_t ground_atom_id);

        bool is_initialized() const;
    };

    struct GeneratorNode
    {
        ActionIter begin;
        ActionIter end;

        GeneratorNode();
        GeneratorNode(ActionIter begin, ActionIter end);
    };

    using Node = std::variant<SelectorNode, GeneratorNode>;

    std::vector<Node> m_nodes;
    std::vector<T> m_elements;

    NodeID build_recursively(const size_t atom_id, size_t const num_atoms, const std::vector<T>& elements);

    void get_applicable_elements_recursively(size_t node_id, const FlatBitset state, std::vector<T>& out_applicable_elements);

public:
    MatchTree();
    MatchTree(const size_t num_atoms, const std::vector<T>& elements);

    void get_applicable_elements(const FlatBitset state, std::vector<T>& out_applicable_elements);

    [[nodiscard]] size_t get_num_nodes() const;

    void print() const;
};

template<typename T>
MatchTree<T>::SelectorNode::SelectorNode(size_t ground_atom_id_) : ground_atom_id(ground_atom_id_), true_succ(-1), false_succ(-1), dontcare_succ(-1)
{
}

template<typename T>
bool MatchTree<T>::SelectorNode::is_initialized() const
{
    return true_succ != (uint32_t) -1 && false_succ != (uint32_t) -1 && dontcare_succ != (uint32_t) -1;
}

template<typename T>
MatchTree<T>::GeneratorNode::GeneratorNode() : begin(0), end(0)
{
}

template<typename T>
MatchTree<T>::GeneratorNode::GeneratorNode(ActionIter begin_, ActionIter end_) : begin(begin_), end(end_)
{
}

template<typename T>
MatchTree<T>::NodeID MatchTree<T>::MatchTree::build_recursively(const size_t atom_id, const size_t num_atoms, const std::vector<T>& elements)
{
    // 1. Base cases:

    // 1.1. There are no more atoms to test or
    // 1.2. there are no elements.
    if ((atom_id == num_atoms) || (elements.empty()))
    {
        const auto node_id = MatchTree::NodeID { m_nodes.size() };
        m_nodes.push_back(MatchTree::GeneratorNode(m_elements.size(), m_elements.size() + elements.size()));
        m_elements.insert(m_elements.end(), elements.begin(), elements.end());

        return node_id;
    }

    // 2. Conquer

    // Partition elements into positive, negative and dontcare depending on how atom_id occurs in precondition
    auto positive_elements = std::vector<T> {};
    auto negative_elements = std::vector<T> {};
    auto dontcare_elements = std::vector<T> {};
    for (const auto& element : elements)
    {
        const bool positive_condition = element.get_applicability_positive_precondition_bitset().get(atom_id);
        const bool negative_condition = element.get_applicability_negative_precondition_bitset().get(atom_id);

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
    const bool must_split = (dontcare_elements.size() != elements.size());
    if (must_split)
    {
        // Top-down creation of nodes, update information after recursion.
        const auto node_id = MatchTree::NodeID { m_nodes.size() };

        m_nodes.push_back(MatchTree::SelectorNode(atom_id));

        const auto true_succ = build_recursively(atom_id + 1, num_atoms, positive_elements);
        const auto false_succ = build_recursively(atom_id + 1, num_atoms, negative_elements);
        const auto dontcare_succ = build_recursively(atom_id + 1, num_atoms, dontcare_elements);
        assert(node_id < true_succ && node_id < false_succ && node_id < dontcare_succ);

        // Update node with computed information
        auto& node = std::get<MatchTree::SelectorNode>(m_nodes[node_id]);
        node.true_succ = true_succ;
        node.false_succ = false_succ;
        node.dontcare_succ = dontcare_succ;
        assert(node.is_initialized());

        return node_id;
    }
    else
    {
        // All elements are dontcares, skip creating a node.
        return build_recursively(atom_id + 1, num_atoms, dontcare_elements);
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

        std::visit(
            [&](const auto& arg)
            {
                using T_ = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T_, MatchTree::GeneratorNode>)
                {
                    std::cout << i << " GeneratorNode(elements=[";
                    for (auto it = m_elements.begin() + arg.begin; it < m_elements.begin() + arg.end; ++it)
                    {
                        std::cout << it->get_id() << ", ";
                    }
                    std::cout << "]" << std::endl;
                }
                else if constexpr (std::is_same_v<T_, MatchTree::SelectorNode>)
                {
                    std::cout << i << " SelectorNode("
                              << "ground_atom_id: " << arg.ground_atom_id << " "
                              << "true_succ: " << arg.true_succ << " "
                              << "false_succ: " << arg.false_succ << " "
                              << "dontcare_succ: " << arg.dontcare_succ << ")" << std::endl;
                }
            },
            node);
    }
}

template<typename T>
MatchTree<T>::MatchTree()
{
    m_nodes.push_back(MatchTree::GeneratorNode());
}

template<typename T>
MatchTree<T>::MatchTree(const size_t num_atoms, const std::vector<T>& elements)
{
    assert(m_nodes.size() == 0);
    const auto root_node_id = build_recursively(m_nodes.size(), num_atoms, elements);
    assert(root_node_id == 0);
}

template<typename T>
void MatchTree<T>::get_applicable_elements_recursively(size_t node_id, const FlatBitset state, std::vector<T>& out_applicable_elements)
{
    auto& node = m_nodes[node_id];

    std::visit(
        [&](const auto& arg)
        {
            using T_ = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T_, MatchTree::GeneratorNode>)
            {
                out_applicable_elements.insert(out_applicable_elements.end(), m_elements.begin() + arg.begin, m_elements.begin() + arg.end);
            }
            else if constexpr (std::is_same_v<T_, MatchTree::SelectorNode>)
            {
                get_applicable_elements_recursively(arg.dontcare_succ, state, out_applicable_elements);

                if (state.get(arg.ground_atom_id))
                {
                    get_applicable_elements_recursively(arg.true_succ, state, out_applicable_elements);
                }
                else
                {
                    get_applicable_elements_recursively(arg.false_succ, state, out_applicable_elements);
                }
            }
        },
        node);
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
