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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
InverseElementGeneratorNode_Perfect<E>::InverseElementGeneratorNode_Perfect(const IInverseNode<E>* parent, std::span<const E*> elements) :
    InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Perfect<E>, E>(parent, elements)
{
}

template<formalism::HasConjunctiveCondition E>
void InverseElementGeneratorNode_Perfect<E>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template class InverseElementGeneratorNode_Perfect<GroundActionImpl>;
template class InverseElementGeneratorNode_Perfect<GroundAxiomImpl>;

template<formalism::HasConjunctiveCondition E>
InverseElementGeneratorNode_Imperfect<E>::InverseElementGeneratorNode_Imperfect(const IInverseNode<E>* parent, std::span<const E*> elements) :
    InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Imperfect<E>, E>(parent, elements)
{
}

template<formalism::HasConjunctiveCondition E>
void InverseElementGeneratorNode_Imperfect<E>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template class InverseElementGeneratorNode_Imperfect<GroundActionImpl>;
template class InverseElementGeneratorNode_Imperfect<GroundAxiomImpl>;

}
