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

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
ElementGeneratorNode_Perfect<Element>::ElementGeneratorNode_Perfect(std::span<const Element*> elements) :
    ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<Element>, Element>(elements)
{
    // m_elements may be empty, e.g., if there are no ground axioms.
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode_Perfect<Element>::generate_applicable_actions_impl(const DenseState& state,
                                                                             std::vector<const Element*>& ref_applicable_elements) const
{
    if constexpr (std::is_same_v<Element, GroundActionImpl>)
    {
        if (!state.get_numeric_variables().empty())
        {
            for (const auto& element : this->m_elements)
            {
                if (is_dynamically_applicable(element, state))
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
    else if constexpr (std::is_same_v<Element, GroundAxiomImpl>)
    {
        ref_applicable_elements.insert(ref_applicable_elements.end(), this->m_elements.begin(), this->m_elements.end());
    }
    else
    {
        static_assert(dependent_false<Element>::value,
                      "ElementGeneratorNode_Perfect<Element>::generate_applicable_actions_impl(state, ref_applicable_elements): Missing implementation for "
                      "Element type.");
    }
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode_Perfect<Element>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template class ElementGeneratorNode_Perfect<GroundActionImpl>;
template class ElementGeneratorNode_Perfect<GroundAxiomImpl>;

template<HasConjunctiveCondition Element>
ElementGeneratorNode_Imperfect<Element>::ElementGeneratorNode_Imperfect(std::span<const Element*> elements) :
    ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<Element>, Element>(elements)
{
    // m_elements may be empty, e.g., if there are no ground axioms.
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode_Imperfect<Element>::generate_applicable_actions_impl(const DenseState& state,
                                                                               std::vector<const Element*>& ref_applicable_elements) const
{
    for (const auto& element : this->m_elements)
    {
        if (is_dynamically_applicable(element, state))
        {
            ref_applicable_elements.push_back(element);
        }
    }
}

template<HasConjunctiveCondition Element>
void ElementGeneratorNode_Imperfect<Element>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template class ElementGeneratorNode_Imperfect<GroundActionImpl>;
template class ElementGeneratorNode_Imperfect<GroundAxiomImpl>;

}
