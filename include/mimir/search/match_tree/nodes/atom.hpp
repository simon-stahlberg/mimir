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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODES_ATOM_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODES_ATOM_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"

namespace mimir::match_tree
{
template<typename Derived_, HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNodeBase : public INode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    GroundAtom<P> m_atom;

public:
    explicit AtomSelectorNodeBase(GroundAtom<P> atom) : m_atom(atom) { assert(m_atom); }

    AtomSelectorNodeBase(const AtomSelectorNodeBase& other) = delete;
    AtomSelectorNodeBase& operator=(const AtomSelectorNodeBase& other) = delete;
    AtomSelectorNodeBase(AtomSelectorNodeBase&& other) = delete;
    AtomSelectorNodeBase& operator=(AtomSelectorNodeBase&& other) = delete;

    GroundAtom<P> get_atom() const { return m_atom; }

    void visit(INodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }
};

/**
 * True False Dontcare combination
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TFX : public AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>
{
private:
    Node<Element> m_true_child;
    Node<Element> m_false_child;
    Node<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_TFX(Node<Element>&& true_child, Node<Element>&& false_child, Node<Element>&& dontcare_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_true_child() const;
    const Node<Element>& get_false_child() const;
    const Node<Element>& get_dontcare_child() const;
};

/**
 * True False combination.
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TF : public AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>
{
private:
    Node<Element> m_true_child;
    Node<Element> m_false_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_TF(Node<Element>&& true_child, Node<Element>&& false_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_true_child() const;
    const Node<Element>& get_false_child() const;
};

/**
 * True Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TX : public AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>
{
private:
    Node<Element> m_true_child;
    Node<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_TX(Node<Element>&& true_child, Node<Element>&& dontcare_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_true_child() const;
    const Node<Element>& get_dontcare_child() const;
};

/**
 * False Dontcare combination.
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_FX : public AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>
{
private:
    Node<Element> m_false_child;
    Node<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_FX(Node<Element>&& false_child, Node<Element>&& dontcare_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_false_child() const;
    const Node<Element>& get_dontcare_child() const;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_T : public AtomSelectorNodeBase<AtomSelectorNode_T<Element, P>, Element, P>
{
private:
    Node<Element> m_true_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_T<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_T<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_T(Node<Element>&& true_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_true_child() const;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_F : public AtomSelectorNodeBase<AtomSelectorNode_F<Element, P>, Element, P>
{
private:
    Node<Element> m_false_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<Element>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_F<Element, P>, Element, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_F<Element, P>, Element, P>::get_atom;

    explicit AtomSelectorNode_F(Node<Element>&& false_child, GroundAtom<P> atom);

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override;

    const Node<Element>& get_false_child() const;
};
}

#endif
