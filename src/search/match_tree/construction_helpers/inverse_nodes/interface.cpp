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

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E>
using Nodes = std::unordered_map<const IInverseNode<E>*, std::pair<size_t, std::string>>;

using Partition = std::map<size_t, std::vector<size_t>>;

using Edges = std::unordered_map<size_t, std::vector<std::pair<size_t, std::string>>>;

template<formalism::HasConjunctiveCondition E>
struct InitializeInverseNodesVisitor : public IInverseNodeVisitor<E>
{
    Nodes<E>& m_nodes;

    InitializeInverseNodesVisitor(Nodes<E>& nodes) : m_nodes(nodes) {}

    void accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_T<E>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<E>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<E>& generator) override;
};

template<formalism::HasConjunctiveCondition E>
struct InitializeInverseEdgesVisitor : public IInverseNodeVisitor<E>
{
    Nodes<E>& m_nodes;
    Edges& m_edges;

    InitializeInverseEdgesVisitor(Nodes<E>& nodes, Edges& edges) : m_nodes(nodes), m_edges(edges) {}

    void accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom) override;
    void accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom) override;
    void accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint) override;
    void accept(const InverseNumericConstraintSelectorNode_T<E>& constraint) override;
    void accept(const InverseElementGeneratorNode_Perfect<E>& generator) override;
    void accept(const InverseElementGeneratorNode_Imperfect<E>& generator) override;
};

/**
 * InitializeInverseNodesVisitor
 */

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

/**
 * InitializeEdges
 */

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_dontcare_child().get()).first, "X");
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_true_child().get()).first, "T");
    atom.get_true_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    visitor.m_edges[visitor.m_nodes.at(&atom).first].emplace_back(visitor.m_nodes.at(atom.get_false_child().get()).first, "F");
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_dontcare_child().get()).first, "X");
    constraint.get_dontcare_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    m_edges[m_nodes.at(&constraint).first].emplace_back(m_nodes.at(constraint.get_true_child().get()).first, "T");
    constraint.get_true_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    // Nothing to be done.
}

template<formalism::HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    // Nothing to be done.
}

/**
 * Printer
 */

template<formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const IInverseNode<E>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const IInverseNode<GroundActionImpl>& element);
template std::ostream& operator<<(std::ostream& out, const IInverseNode<GroundAxiomImpl>& element);
}

namespace mimir
{
template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<E>& element)
{
    auto nodes = mimir::search::match_tree::Nodes<E> {};
    auto edges = mimir::search::match_tree::Edges {};

    auto visitor1 = mimir::search::match_tree::InitializeInverseNodesVisitor(nodes);
    element.visit(visitor1);

    auto visitor2 = mimir::search::match_tree::InitializeInverseEdgesVisitor(nodes, edges);
    element.visit(visitor2);

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

template std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundActionImpl>& element);
template std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundAxiomImpl>& element);
}