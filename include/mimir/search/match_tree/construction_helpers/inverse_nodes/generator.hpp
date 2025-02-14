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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_GENERATOR_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_GENERATOR_HPP_

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::match_tree
{
template<typename Derived_, HasConjunctiveCondition Element>
class InverseElementGeneratorNodeBase : public IInverseNode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    // Final elements
    std::span<const Element*> m_elements;

public:
    InverseElementGeneratorNodeBase(const IInverseNode<Element>* parent, std::span<const Element*> elements) :
        IInverseNode<Element>(parent, SplitList {}),
        m_elements(elements)
    {
    }

    InverseElementGeneratorNodeBase(const InverseElementGeneratorNodeBase& other) = delete;
    InverseElementGeneratorNodeBase& operator=(const InverseElementGeneratorNodeBase& other) = delete;
    InverseElementGeneratorNodeBase(InverseElementGeneratorNodeBase&& other) = delete;
    InverseElementGeneratorNodeBase& operator=(InverseElementGeneratorNodeBase&& other) = delete;

    void visit(IInverseNodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }

    std::span<const Element*> get_elements() const { return m_elements; }
};

template<HasConjunctiveCondition Element>
class InverseElementGeneratorNode_Perfect : public InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Perfect<Element>, Element>
{
private:
    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Perfect<Element>, Element>;

public:
    using InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Perfect<Element>, Element>::get_elements;

    InverseElementGeneratorNode_Perfect(const IInverseNode<Element>* parent, std::span<const Element*> elements);
};

template<HasConjunctiveCondition Element>
class InverseElementGeneratorNode_Imperfect : public InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Imperfect<Element>, Element>
{
private:
    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Imperfect<Element>, Element>;

public:
    using InverseElementGeneratorNodeBase<InverseElementGeneratorNode_Imperfect<Element>, Element>::get_elements;

    InverseElementGeneratorNode_Imperfect(const IInverseNode<Element>* parent, std::span<const Element*> elements);
};
}

#endif
