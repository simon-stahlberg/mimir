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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_INTERFACE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_INTERFACE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"

#include <map>
#include <unordered_map>

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class IInverseNodeVisitor
{
public:
    virtual void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) = 0;
    virtual void accept(const InverseAtomSelectorNode<Element, Derived>& atom) = 0;
    virtual void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) = 0;
    virtual void accept(const InverseElementGeneratorNode<Element>& generator) = 0;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class IInverseNode
{
protected:
    const IInverseNode<Element>* m_parent;  ///< Raw pointer to parent node, we must ensure that root is kept alive!
    SplitList m_useless_splits;             ///< Splits that were detected as useless.
    size_t m_root_distance;                 ///< The distance to the root node.

public:
    IInverseNode(const IInverseNode<Element>* parent, SplitList useless_splits, size_t root_distance) :
        m_parent(parent),
        m_useless_splits(std::move(useless_splits)),
        m_root_distance(root_distance)
    {
    }
    virtual ~IInverseNode() = default;

    virtual const IInverseNode<Element>* get_parent() const { return m_parent; }
    virtual const SplitList& get_useless_splits() const { return m_useless_splits; }
    virtual size_t get_root_distance() const { return m_root_distance; }

    virtual void visit(IInverseNodeVisitor<Element>& visitor) const = 0;
};

template<HasConjunctiveCondition Element>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<const IInverseNode<Element>&, DotPrinterTag>& tree)
{
    using Nodes = std::unordered_map<const IInverseNode<Element>*, std::pair<size_t, std::string>>;
    using Partition = std::map<size_t, std::vector<size_t>>;
    using Edges = std::unordered_map<size_t, std::vector<std::pair<size_t, std::string>>>;

    struct InitializeNodes : public IInverseNodeVisitor<Element>
    {
        Nodes& m_nodes;

        InitializeNodes(Nodes& nodes) : m_nodes(nodes) {}

        void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
        {
            m_nodes.emplace(&atom, std::make_pair(m_nodes.size(), to_string(atom.get_atom())));
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
        {
            m_nodes.emplace(&atom, std::make_pair(m_nodes.size(), to_string(atom.get_atom())));
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
        {
            m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
            constraint.get_true_child()->visit(*this);
            constraint.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseElementGeneratorNode<Element>& generator) override
        {
            m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
        }
    };

    struct InitializePartitionVisitor : public IInverseNodeVisitor<Element>
    {
        Nodes& m_nodes;
        Partition& m_partition;

        InitializePartitionVisitor(Nodes& nodes, Partition& partition) : m_nodes(nodes), m_partition(partition) {}

        void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
        {
            m_partition[atom.get_root_distance()].push_back(m_nodes.at(&atom).first);
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
        {
            m_partition[atom.get_root_distance()].push_back(m_nodes.at(&atom).first);
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
        {
            m_partition[constraint.get_root_distance()].push_back(m_nodes.at(&constraint).first);
            constraint.get_true_child()->visit(*this);
            constraint.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseElementGeneratorNode<Element>& generator) override
        {
            m_partition[generator.get_root_distance()].push_back(m_nodes.at(&generator).first);
        }
    };

    struct InitializeEdgesVisitor : public IInverseNodeVisitor<Element>
    {
        Nodes& m_nodes;
        Edges& m_edges;

        InitializeEdgesVisitor(Nodes& nodes, Edges& edges) : m_nodes(nodes), m_edges(edges) {}

        void accept(const InverseAtomSelectorNode<Element, Fluent>& atom) override
        {
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_true_child().get()).first, "T");
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_false_child().get()).first, "F");
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_dontcare_child().get()).first, "X");
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseAtomSelectorNode<Element, Derived>& atom) override
        {
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_true_child().get()).first, "T");
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_false_child().get()).first, "F");
            m_edges[m_nodes.at(&atom).first].emplace_back(m_nodes.at(atom.get_dontcare_child().get()).first, "X");
            atom.get_true_child()->visit(*this);
            atom.get_false_child()->visit(*this);
            atom.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override
        {
            m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
            m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_dontcare_child().get()).first, "X");
            constraint.get_true_child()->visit(*this);
            constraint.get_dontcare_child()->visit(*this);
        }
        void accept(const InverseElementGeneratorNode<Element>& generator) override {}
    };

    auto nodes = Nodes {};
    auto partition = Partition {};
    auto edges = Edges {};

    auto& [root, tag] = tree;

    auto visitor1 = InitializeNodes(nodes);
    root.visit(visitor1);

    auto visitor2 = InitializePartitionVisitor(nodes, partition);
    root.visit(visitor2);

    auto visitor3 = InitializeEdgesVisitor(nodes, edges);
    root.visit(visitor3);

    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

    /* Rank definitions */
    for (const auto& [distance, nodes] : partition)
    {
        out << "{ rank=same; ";
        for (const auto& node : nodes)
        {
            out << "n" << node << "; ";
        }
        out << "}\n";
    }
    out << "\n";

    /* Node definitions */
    for (const auto& [node_ptr, node_data] : nodes)
    {
        auto& [index, label] = node_data;

        out << "n" << index << " [label=\"" << label << "\"];\n";
    }
    out << "\n";

    /* Edge definitions */
    for (const auto& [src, edges] : edges)
    {
        for (const auto& [dst, label] : edges)
        {
            out << "n" << src << " -> " << "n" << dst << " [label=\"" << label << "\"];\n";
        }
    }
    out << "\n";

    /* Invisible edge definitions to control layering */
    for (auto it = partition.begin(); it != partition.end(); ++it)
    {
        const auto next = std::next(it);
        if (next == partition.end())
        {
            break;  // skip last element
        }

        // draw invisible edges to nodes in next partition
        for (const auto& node : it->second)
        {
            for (const auto& node2 : next->second)
            {
                // A -> X [style=invis];
                out << "n" << node << " -> " << "n" << node2 << " [style=invis];\n";
            }
        }
    }

    out << "}\n";  // end graph

    return out;
}
}

#endif
