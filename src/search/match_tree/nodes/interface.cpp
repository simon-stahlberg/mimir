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

#include "mimir/search/match_tree/nodes/interface.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
using Nodes = std::unordered_map<const INode<Element>*, std::pair<size_t, std::string>>;

using Edges = std::unordered_map<size_t, std::vector<std::pair<size_t, std::string>>>;

template<HasConjunctiveCondition Element>
struct InitializeNodesVisitor : public INodeVisitor<Element>
{
    Nodes<Element>& m_nodes;

    InitializeNodesVisitor(Nodes<Element>& nodes) : m_nodes(nodes) {}

    void accept(const AtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const NumericConstraintSelectorNode<Element>& constraint) override;
    void accept(const ElementGeneratorNode<Element>& generator) override;
};

template<HasConjunctiveCondition Element>
struct InitializeEdgesVisitor : public INodeVisitor<Element>
{
    Nodes<Element>& m_nodes;
    Edges& m_edges;

    InitializeEdgesVisitor(Nodes<Element>& nodes, Edges& edges) : m_nodes(nodes), m_edges(edges) {}

    void accept(const AtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const NumericConstraintSelectorNode<Element>& constraint) override;
    void accept(const ElementGeneratorNode<Element>& generator) override;
};

/**
 * InitializeNodes
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_T<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeNodesVisitor<Element>& visitor, const AtomSelectorNode_F<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const AtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const NumericConstraintSelectorNode<Element>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    if (constraint.get_true_child())
        constraint.get_true_child()->visit(*this);
    if (constraint.get_dontcare_child())
        constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeNodesVisitor<Element>::accept(const ElementGeneratorNode<Element>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

/**
 * InitializeEdges
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_T<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeEdgesVisitor<Element>& visitor, const AtomSelectorNode_F<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const AtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const NumericConstraintSelectorNode<Element>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_dontcare_child().get()).first, "X");
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeEdgesVisitor<Element>::accept(const ElementGeneratorNode<Element>& generator)
{
    // Nothing to be done.
}

/**
 * Printer
 */

template<HasConjunctiveCondition Element>
std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<Element>&, DotPrinterTag>& tree)
{
    auto nodes = Nodes<Element> {};
    auto edges = Edges {};

    auto& [root, tag] = tree;

    auto visitor1 = InitializeNodesVisitor(nodes);
    root->visit(visitor1);

    auto visitor2 = InitializeEdgesVisitor(nodes, edges);
    root->visit(visitor2);

    out << "digraph Tree {\n"
           "rankdir=TB;\n\n";

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

    out << "}\n";  // end graph

    return out;
}

template std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<GroundActionImpl>&, DotPrinterTag>& tree);
template std::ostream& operator<<(std::ostream& out, const std::tuple<const Node<GroundAxiomImpl>&, DotPrinterTag>& tree);
}