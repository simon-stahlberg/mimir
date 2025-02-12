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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_GENERATOR_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_GENERATOR_HPP_

#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<typename Derived_, HasConjunctiveCondition Element>
class ElementGeneratorNodeBase : public INode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    std::span<const Element*> m_elements;

public:
    explicit ElementGeneratorNodeBase(std::span<const Element*> elements) : m_elements(elements) {}

    ElementGeneratorNodeBase(const ElementGeneratorNodeBase& other) = delete;
    ElementGeneratorNodeBase& operator=(const ElementGeneratorNodeBase& other) = delete;
    ElementGeneratorNodeBase(ElementGeneratorNodeBase&& other) = delete;
    ElementGeneratorNodeBase& operator=(ElementGeneratorNodeBase&& other) = delete;

    void generate_applicable_actions(const DenseState& state,
                                     std::vector<const INode<Element>*>&,
                                     std::vector<const Element*>& ref_applicable_elements) const override
    {
        self().generate_applicable_actions_impl(state, ref_applicable_elements);
    }

    std::span<const Element*> get_elements() const { return m_elements; }

    void visit(INodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }
};

template<HasConjunctiveCondition Element>
class ElementGeneratorNode_Perfect : public ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<Element>, Element>
{
private:
    void generate_applicable_actions_impl(const DenseState& state, std::vector<const Element*>& ref_applicable_elements) const;

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<Element>, Element>;

public:
    using ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<Element>, Element>::get_elements;

    explicit ElementGeneratorNode_Perfect(std::span<const Element*> elements);
};

template<HasConjunctiveCondition Element>
class ElementGeneratorNode_Imperfect : public ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<Element>, Element>
{
private:
    void generate_applicable_actions_impl(const DenseState& state, std::vector<const Element*>& ref_applicable_elements) const;

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<Element>, Element>;

public:
    using ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<Element>, Element>::get_elements;

    explicit ElementGeneratorNode_Imperfect(std::span<const Element*> elements);
};
}

#endif
