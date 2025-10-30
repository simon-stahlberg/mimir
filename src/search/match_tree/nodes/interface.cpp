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

#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

/**
 * InitializeNodes
 */

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_T<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeNodesVisitor<E>& visitor, const AtomSelectorNode_F<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TFX<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TF<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TF<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TX<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_FX<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_FX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_T<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_T<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_F<GroundActionImpl, FluentTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_F<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TFX<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TF<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TX<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_FX<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_T<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_T<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const AtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_F<GroundActionImpl, DerivedTag>& atom);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_F<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const NumericConstraintSelectorNode_TX<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const NumericConstraintSelectorNode_TX<GroundActionImpl>& constraint);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const NumericConstraintSelectorNode_TX<GroundAxiomImpl>& constraint);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const NumericConstraintSelectorNode_T<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const NumericConstraintSelectorNode_T<GroundActionImpl>& constraint);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const NumericConstraintSelectorNode_T<GroundAxiomImpl>& constraint);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const ElementGeneratorNode_Perfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const ElementGeneratorNode_Perfect<GroundActionImpl>& generator);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const ElementGeneratorNode_Perfect<GroundAxiomImpl>& generator);

template<formalism::HasConjunctiveCondition E>
void InitializeNodesVisitor<E>::accept(const ElementGeneratorNode_Imperfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template void InitializeNodesVisitor<GroundActionImpl>::accept(const ElementGeneratorNode_Imperfect<GroundActionImpl>& generator);
template void InitializeNodesVisitor<GroundAxiomImpl>::accept(const ElementGeneratorNode_Imperfect<GroundAxiomImpl>& generator);

/**
 * InitializeEdges
 */

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_T<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeEdgesVisitor<E>& visitor, const AtomSelectorNode_F<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TFX<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TF<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TF<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TX<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_FX<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_FX<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_T<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_T<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_F<GroundActionImpl, FluentTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_F<GroundAxiomImpl, FluentTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TFX<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TF<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_TX<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_FX<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_T<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_T<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const AtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const AtomSelectorNode_F<GroundActionImpl, DerivedTag>& atom);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const AtomSelectorNode_F<GroundAxiomImpl, DerivedTag>& atom);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const NumericConstraintSelectorNode_TX<E>& constraint)
{
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_true_child().get()).first)] = "T";
    constraint.get_true_child()->visit(*this);
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_dontcare_child().get()).first)] = "X";
    constraint.get_dontcare_child()->visit(*this);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const NumericConstraintSelectorNode_TX<GroundActionImpl>& constraint);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const NumericConstraintSelectorNode_TX<GroundAxiomImpl>& constraint);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const NumericConstraintSelectorNode_T<E>& constraint)
{
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_true_child().get()).first)] = "T";
    constraint.get_true_child()->visit(*this);
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const NumericConstraintSelectorNode_T<GroundActionImpl>& constraint);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const NumericConstraintSelectorNode_T<GroundAxiomImpl>& constraint);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const ElementGeneratorNode_Perfect<E>& generator)
{
    // Nothing to be done.
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const ElementGeneratorNode_Perfect<GroundActionImpl>& generator);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const ElementGeneratorNode_Perfect<GroundAxiomImpl>& generator);

template<formalism::HasConjunctiveCondition E>
void InitializeEdgesVisitor<E>::accept(const ElementGeneratorNode_Imperfect<E>& generator)
{
    // Nothing to be done.
}

template void InitializeEdgesVisitor<GroundActionImpl>::accept(const ElementGeneratorNode_Imperfect<GroundActionImpl>& generator);
template void InitializeEdgesVisitor<GroundAxiomImpl>::accept(const ElementGeneratorNode_Imperfect<GroundAxiomImpl>& generator);
}
