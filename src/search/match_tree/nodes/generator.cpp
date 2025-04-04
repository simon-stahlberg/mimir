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

#include "mimir/search/match_tree/nodes/generator.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/dense_state.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{
template<formalism::HasConjunctiveCondition E>
ElementGeneratorNode_Perfect<E>::ElementGeneratorNode_Perfect(std::span<const E*> elements) :
    ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<E>, E>(elements)
{
    // m_elements may be empty, e.g., if there are no ground axioms.
}

template<formalism::HasConjunctiveCondition E>
void ElementGeneratorNode_Perfect<E>::generate_applicable_actions_impl(const DenseState& state,
                                                                       const ProblemImpl& problem,
                                                                       std::vector<const E*>& ref_applicable_elements) const
{
    if constexpr (std::is_same_v<E, GroundActionImpl>)
    {
        if (!state.get_numeric_variables().empty())
        {
            for (const auto& element : this->m_elements)
            {
                if (is_dynamically_applicable(element, problem, state))
                {
                    ref_applicable_elements.push_back(element);
                }
            }
        }
        else
        {
            ref_applicable_elements.insert(ref_applicable_elements.end(), this->m_elements.begin(), this->m_elements.end());
        }
    }
    else if constexpr (std::is_same_v<E, GroundAxiomImpl>)
    {
        ref_applicable_elements.insert(ref_applicable_elements.end(), this->m_elements.begin(), this->m_elements.end());
    }
    else
    {
        static_assert(dependent_false<E>::value,
                      "ElementGeneratorNode_Perfect<E>::generate_applicable_actions_impl(state, ref_applicable_elements): Missing implementation for "
                      "Element type.");
    }
}

template<formalism::HasConjunctiveCondition E>
void ElementGeneratorNode_Perfect<E>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template class ElementGeneratorNode_Perfect<GroundActionImpl>;
template class ElementGeneratorNode_Perfect<GroundAxiomImpl>;

template<formalism::HasConjunctiveCondition E>
ElementGeneratorNode_Imperfect<E>::ElementGeneratorNode_Imperfect(std::span<const E*> elements) :
    ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<E>, E>(elements)
{
    // m_elements may be empty, e.g., if there are no ground axioms.
}

template<formalism::HasConjunctiveCondition E>
void ElementGeneratorNode_Imperfect<E>::generate_applicable_actions_impl(const DenseState& state,
                                                                         const ProblemImpl& problem,
                                                                         std::vector<const E*>& ref_applicable_elements) const
{
    for (const auto& element : this->m_elements)
    {
        if (is_dynamically_applicable(element, problem, state))
        {
            ref_applicable_elements.push_back(element);
        }
    }
}

template<formalism::HasConjunctiveCondition E>
void ElementGeneratorNode_Imperfect<E>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template class ElementGeneratorNode_Imperfect<GroundActionImpl>;
template class ElementGeneratorNode_Imperfect<GroundAxiomImpl>;

}
