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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
using Nodes = std::unordered_map<const IInverseNode<Element>*, std::pair<size_t, std::string>>;

using Partition = std::map<size_t, std::vector<size_t>>;

using Edges = std::unordered_map<size_t, std::vector<std::pair<size_t, std::string>>>;

template<HasConjunctiveCondition Element>
struct InitializeNodes : public IInverseNodeVisitor<Element>
{
    Nodes<Element>& m_nodes;

    InitializeNodes(Nodes<Element>& nodes) : m_nodes(nodes) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode<Element>& generator) override;
};

template<HasConjunctiveCondition Element>
struct InitializePartitionVisitor : public IInverseNodeVisitor<Element>
{
    Nodes<Element>& m_nodes;
    Partition& m_partition;

    InitializePartitionVisitor(Nodes<Element>& nodes, Partition& partition) : m_nodes(nodes), m_partition(partition) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode<Element>& generator) override;
};

template<HasConjunctiveCondition Element>
struct InitializeEdgesVisitor : public IInverseNodeVisitor<Element>
{
    Nodes<Element>& m_nodes;
    Edges& m_edges;

    InitializeEdgesVisitor(Nodes<Element>& nodes, Edges& edges) : m_nodes(nodes), m_edges(edges) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode<Element>& generator) override;
};

/**
 * InitializeNodes
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodes<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodes<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodes<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodes<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseNumericConstraintSelectorNode<Element>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    if (constraint.get_true_child())
        constraint.get_true_child()->visit(*this);
    if (constraint.get_dontcare_child())
        constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeNodes<Element>::accept(const InverseElementGeneratorNode<Element>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

/**
 * InitializePartition
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializePartitionVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_partition[atom.get_root_distance()].push_back(visitor.m_nodes.at(&atom).first);
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializePartitionVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_partition[atom.get_root_distance()].push_back(visitor.m_nodes.at(&atom).first);
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializePartitionVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_partition[atom.get_root_distance()].push_back(visitor.m_nodes.at(&atom).first);
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializePartitionVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_partition[atom.get_root_distance()].push_back(visitor.m_nodes.at(&atom).first);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseNumericConstraintSelectorNode<Element>& constraint)
{
    m_partition[constraint.get_root_distance()].push_back(m_nodes.at(&constraint).first);
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializePartitionVisitor<Element>::accept(const InverseElementGeneratorNode<Element>& generator)
{
    m_partition[generator.get_root_distance()].push_back(m_nodes.at(&generator).first);
}

/**
 * InitializeEdges
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseNumericConstraintSelectorNode<Element>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_dontcare_child().get()).first, "X");
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const InverseElementGeneratorNode<Element>& generator)
{
    // Nothing to be done.
}

/**
 * Printer
 */

template<HasConjunctiveCondition Element>
std::ostream& operator<<(std::ostream& out, const std::tuple<const IInverseNode<Element>&, DotPrinterTag>& tree)
{
    auto nodes = Nodes<Element> {};
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

template std::ostream& operator<<(std::ostream& out, const std::tuple<const IInverseNode<GroundActionImpl>&, DotPrinterTag>& tree);
template std::ostream& operator<<(std::ostream& out, const std::tuple<const IInverseNode<GroundAxiomImpl>&, DotPrinterTag>& tree);
}