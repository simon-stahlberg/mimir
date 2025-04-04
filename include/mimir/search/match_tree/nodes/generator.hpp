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

namespace mimir::search::match_tree
{
template<typename Derived_, formalism::HasConjunctiveCondition E>
class ElementGeneratorNodeBase : public INode<E>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    std::span<const E*> m_elements;

public:
    explicit ElementGeneratorNodeBase(std::span<const E*> elements) : m_elements(elements) {}

    ElementGeneratorNodeBase(const ElementGeneratorNodeBase& other) = delete;
    ElementGeneratorNodeBase& operator=(const ElementGeneratorNodeBase& other) = delete;
    ElementGeneratorNodeBase(ElementGeneratorNodeBase&& other) = delete;
    ElementGeneratorNodeBase& operator=(ElementGeneratorNodeBase&& other) = delete;

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>&,
                                     std::vector<const E*>& ref_applicable_elements) const override
    {
        self().generate_applicable_actions_impl(state, problem, ref_applicable_elements);
    }

    std::span<const E*> get_elements() const { return m_elements; }

    void visit(INodeVisitor<E>& visitor) const override { self().visit_impl(visitor); }
};

template<formalism::HasConjunctiveCondition E>
class ElementGeneratorNode_Perfect : public ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<E>, E>
{
private:
    void generate_applicable_actions_impl(const DenseState& state, const formalism::ProblemImpl& problem, std::vector<const E*>& ref_applicable_elements) const;

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<E>, E>;

public:
    using ElementGeneratorNodeBase<ElementGeneratorNode_Perfect<E>, E>::get_elements;

    explicit ElementGeneratorNode_Perfect(std::span<const E*> elements);
};

template<formalism::HasConjunctiveCondition E>
class ElementGeneratorNode_Imperfect : public ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<E>, E>
{
private:
    void generate_applicable_actions_impl(const DenseState& state, const formalism::ProblemImpl& problem, std::vector<const E*>& ref_applicable_elements) const;

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<E>, E>;

public:
    using ElementGeneratorNodeBase<ElementGeneratorNode_Imperfect<E>, E>::get_elements;

    explicit ElementGeneratorNode_Imperfect(std::span<const E*> elements);
};
}

#endif
