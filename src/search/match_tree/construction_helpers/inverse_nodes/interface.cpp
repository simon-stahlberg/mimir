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

#include "mimir/formalism/formatter.hpp"
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

/**
 * InitializeInverseNodesVisitor
 */

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseNodesVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    visitor.m_nodes.emplace(&atom, std::make_pair(visitor.m_nodes.size(), to_string(atom.get_atom())));
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TFX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TF<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TF<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_FX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_FX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_T<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_T<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_F<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_F<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TFX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TF<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_FX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_T<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_T<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_F<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_F<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseNumericConstraintSelectorNode_TX<GroundActionImpl>& constraint);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseNumericConstraintSelectorNode_TX<GroundAxiomImpl>& constraint);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    m_nodes.emplace(&constraint, std::make_pair(m_nodes.size(), to_string(constraint.get_constraint())));
    constraint.get_true_child()->visit(*this);
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseNumericConstraintSelectorNode_T<GroundActionImpl>& constraint);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseNumericConstraintSelectorNode_T<GroundAxiomImpl>& constraint);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseElementGeneratorNode_Perfect<GroundActionImpl>& generator);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseElementGeneratorNode_Perfect<GroundAxiomImpl>& generator);

template<HasConjunctiveCondition E>
void InitializeInverseNodesVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    m_nodes.emplace(&generator, std::make_pair(m_nodes.size(), std::to_string(generator.get_elements().size())));
}

template void InitializeInverseNodesVisitor<GroundActionImpl>::accept(const InverseElementGeneratorNode_Imperfect<GroundActionImpl>& generator);
template void InitializeInverseNodesVisitor<GroundAxiomImpl>::accept(const InverseElementGeneratorNode_Imperfect<GroundAxiomImpl>& generator);

/**
 * InitializeEdges
 */

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TFX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TF<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_TX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_FX<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_dontcare_child().get()).first)] = "X";
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_T<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_true_child().get()).first)] = "T";
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(InitializeInverseEdgesVisitor<E>& visitor, const InverseAtomSelectorNode_F<E, P>& atom)
{
    visitor.m_edges[std::make_pair(visitor.m_nodes.at(&atom).first, visitor.m_nodes.at(atom.get_false_child().get()).first)] = "F";
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TFX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TF<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TF<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_FX<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_FX<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_T<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_T<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_F<GroundActionImpl, FluentTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_F<GroundAxiomImpl, FluentTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TFX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TF<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_TX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_FX<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_T<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_T<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseAtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseAtomSelectorNode_F<GroundActionImpl, DerivedTag>& atom);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseAtomSelectorNode_F<GroundAxiomImpl, DerivedTag>& atom);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_TX<E>& constraint)
{
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_true_child().get()).first)] = "T";
    constraint.get_true_child()->visit(*this);
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_dontcare_child().get()).first)] = "X";
    constraint.get_dontcare_child()->visit(*this);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseNumericConstraintSelectorNode_TX<GroundActionImpl>& constraint);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseNumericConstraintSelectorNode_TX<GroundAxiomImpl>& constraint);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseNumericConstraintSelectorNode_T<E>& constraint)
{
    m_edges[std::make_pair(m_nodes.at(&constraint).first, m_nodes.at(constraint.get_true_child().get()).first)] = "T";
    constraint.get_true_child()->visit(*this);
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseNumericConstraintSelectorNode_T<GroundActionImpl>& constraint);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseNumericConstraintSelectorNode_T<GroundAxiomImpl>& constraint);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseElementGeneratorNode_Perfect<E>& generator)
{
    // Nothing to be done.
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseElementGeneratorNode_Perfect<GroundActionImpl>& generator);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseElementGeneratorNode_Perfect<GroundAxiomImpl>& generator);

template<HasConjunctiveCondition E>
void InitializeInverseEdgesVisitor<E>::accept(const InverseElementGeneratorNode_Imperfect<E>& generator)
{
    // Nothing to be done.
}

template void InitializeInverseEdgesVisitor<GroundActionImpl>::accept(const InverseElementGeneratorNode_Imperfect<GroundActionImpl>& generator);
template void InitializeInverseEdgesVisitor<GroundAxiomImpl>::accept(const InverseElementGeneratorNode_Imperfect<GroundAxiomImpl>& generator);
}
