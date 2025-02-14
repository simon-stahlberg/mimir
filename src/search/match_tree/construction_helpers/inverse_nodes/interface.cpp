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
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
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
struct InitializeInverseNodesVisitor : public IInverseNodeVisitor<Element>
{
    Nodes<Element>& m_nodes;

    InitializeInverseNodesVisitor(Nodes<Element>& nodes) : m_nodes(nodes) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<Element>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<Element>& generator) override;
};

template<HasConjunctiveCondition Element>
struct InitializeInverseEdgesVisitor : public IInverseNodeVisitor<Element>
{
    Nodes<Element>& m_nodes;
    Edges& m_edges;

    InitializeInverseEdgesVisitor(Nodes<Element>& nodes, Edges& edges) : m_nodes(nodes), m_edges(edges) {}

    void accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const InverseAtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<Element>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<Element>& generator) override;
};

/**
 * InitializeInverseNodesVisitor
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_T<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseNodesVisitor<Element>& visitor, const InverseAtomSelectorNode_F<Element, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseElementGeneratorNode_Perfect<Element>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template<HasConjunctiveCondition Element>
void InitializeInverseNodesVisitor<Element>::accept(const InverseElementGeneratorNode_Imperfect<Element>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

/**
 * InitializeEdges
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TFX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TF<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_TX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_FX<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_T<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(InitializeInverseEdgesVisitor<Element>& visitor, const InverseAtomSelectorNode_F<Element, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseAtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_TX<Element>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_dontcare_child().get()).first, "X");
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseNumericConstraintSelectorNode_T<Element>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseElementGeneratorNode_Perfect<Element>& generator)
{
    // Nothing to be done.
}

template<HasConjunctiveCondition Element>
void InitializeInverseEdgesVisitor<Element>::accept(const InverseElementGeneratorNode_Imperfect<Element>& generator)
{
    // Nothing to be done.
}

/**
 * Printer
 */

template<HasConjunctiveCondition Element>
std::ostream& operator<<(std::ostream& out, const std::tuple<const InverseNode<Element>&, DotPrinterTag>& tree)
{
    auto nodes = Nodes<Element> {};
    auto edges = Edges {};

    auto& [root, tag] = tree;

    auto visitor1 = InitializeInverseNodesVisitor(nodes);
    root->visit(visitor1);

    auto visitor2 = InitializeInverseEdgesVisitor(nodes, edges);
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

template std::ostream& operator<<(std::ostream& out, const std::tuple<const InverseNode<GroundActionImpl>&, DotPrinterTag>& tree);
template std::ostream& operator<<(std::ostream& out, const std::tuple<const InverseNode<GroundAxiomImpl>&, DotPrinterTag>& tree);
}