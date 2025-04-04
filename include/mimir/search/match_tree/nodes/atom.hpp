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

namespace mimir::search::match_tree
{
template<typename Derived_, formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNodeBase : public INode<E>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    formalism::GroundAtom<P> m_atom;

public:
    explicit AtomSelectorNodeBase(formalism::GroundAtom<P> atom) : m_atom(atom) { assert(m_atom); }

    AtomSelectorNodeBase(const AtomSelectorNodeBase& other) = delete;
    AtomSelectorNodeBase& operator=(const AtomSelectorNodeBase& other) = delete;
    AtomSelectorNodeBase(AtomSelectorNodeBase&& other) = delete;
    AtomSelectorNodeBase& operator=(AtomSelectorNodeBase&& other) = delete;

    formalism::GroundAtom<P> get_atom() const { return m_atom; }

    void visit(INodeVisitor<E>& visitor) const override { self().visit_impl(visitor); }
};

/**
 * True False Dontcare combination
 */

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TFX : public AtomSelectorNodeBase<AtomSelectorNode_TFX<E, P>, E, P>
{
private:
    Node<E> m_true_child;
    Node<E> m_false_child;
    Node<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TFX<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TFX<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_TFX(Node<E>&& true_child, Node<E>&& false_child, Node<E>&& dontcare_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
    const Node<E>& get_false_child() const;
    const Node<E>& get_dontcare_child() const;
};

/**
 * True False combination.
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TF : public AtomSelectorNodeBase<AtomSelectorNode_TF<E, P>, E, P>
{
private:
    Node<E> m_true_child;
    Node<E> m_false_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TF<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TF<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_TF(Node<E>&& true_child, Node<E>&& false_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
    const Node<E>& get_false_child() const;
};

/**
 * True Dontcare combination
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_TX : public AtomSelectorNodeBase<AtomSelectorNode_TX<E, P>, E, P>
{
private:
    Node<E> m_true_child;
    Node<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_TX<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_TX<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_TX(Node<E>&& true_child, Node<E>&& dontcare_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
    const Node<E>& get_dontcare_child() const;
};

/**
 * False Dontcare combination.
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_FX : public AtomSelectorNodeBase<AtomSelectorNode_FX<E, P>, E, P>
{
private:
    Node<E> m_false_child;
    Node<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_FX<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_FX<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_FX(Node<E>&& false_child, Node<E>&& dontcare_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_false_child() const;
    const Node<E>& get_dontcare_child() const;
};

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_T : public AtomSelectorNodeBase<AtomSelectorNode_T<E, P>, E, P>
{
private:
    Node<E> m_true_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_T<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_T<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_T(Node<E>&& true_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_true_child() const;
};

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class AtomSelectorNode_F : public AtomSelectorNodeBase<AtomSelectorNode_F<E, P>, E, P>
{
private:
    Node<E> m_false_child;

    /* Implement interface*/

    void visit_impl(INodeVisitor<E>& visitor) const;

    friend class AtomSelectorNodeBase<AtomSelectorNode_F<E, P>, E, P>;

public:
    using AtomSelectorNodeBase<AtomSelectorNode_F<E, P>, E, P>::get_atom;

    explicit AtomSelectorNode_F(Node<E>&& false_child, formalism::GroundAtom<P> atom);

    void generate_applicable_actions(const DenseState& state,
                                     const formalism::ProblemImpl& problem,
                                     std::vector<const INode<E>*>& ref_applicable_nodes,
                                     std::vector<const E*>&) const override;

    const Node<E>& get_false_child() const;
};
}

#endif
