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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_ATOM_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_INVERSE_NODES_ATOM_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"

namespace mimir::search::match_tree
{
template<typename Derived_, formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNodeBase : public IInverseNode<E>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    formalism::GroundAtom<P> m_atom;

public:
    InverseAtomSelectorNodeBase(const IInverseNode<E>* parent, SplitList useless_splits, formalism::GroundAtom<P> atom) :
        IInverseNode<E>(parent, std::move(useless_splits)),
        m_atom(atom)
    {
        assert(m_atom);
    }
    InverseAtomSelectorNodeBase(const InverseAtomSelectorNodeBase& other) = delete;
    InverseAtomSelectorNodeBase& operator=(const InverseAtomSelectorNodeBase& other) = delete;
    InverseAtomSelectorNodeBase(InverseAtomSelectorNodeBase&& other) = delete;
    InverseAtomSelectorNodeBase& operator=(InverseAtomSelectorNodeBase&& other) = delete;

    void visit(IInverseNodeVisitor<E>& visitor) const override { self().visit_impl(visitor); }

    formalism::GroundAtom<P> get_atom() const { return m_atom; };
};

/**
 * True False Dontcare combination
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TFX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;
    std::span<const E*> m_false_elements;
    std::span<const E*> m_dontcare_elements;

    InverseNode<E> m_true_child;
    InverseNode<E> m_false_child;
    InverseNode<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_TFX(const IInverseNode<E>* parent,
                                         SplitList useless_splits,
                                         formalism::GroundAtom<P> atom,
                                         std::span<const E*> true_elements,
                                         std::span<const E*> false_elements,
                                         std::span<const E*> dontcare_elements);

    std::span<const E*> get_true_elements() const;
    std::span<const E*> get_false_elements() const;
    std::span<const E*> get_dontcare_elements() const;

    InverseNode<E>& get_true_child();
    InverseNode<E>& get_false_child();
    InverseNode<E>& get_dontcare_child();

    const InverseNode<E>& get_true_child() const;
    const InverseNode<E>& get_false_child() const;
    const InverseNode<E>& get_dontcare_child() const;
};

/**
 * True False combination
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TF : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;
    std::span<const E*> m_false_elements;

    InverseNode<E> m_true_child;
    InverseNode<E> m_false_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_TF(const IInverseNode<E>* parent,
                                        SplitList useless_splits,
                                        formalism::GroundAtom<P> atom,
                                        std::span<const E*> true_elements,
                                        std::span<const E*> false_elements);

    std::span<const E*> get_true_elements() const;
    std::span<const E*> get_false_elements() const;

    InverseNode<E>& get_true_child();
    InverseNode<E>& get_false_child();

    const InverseNode<E>& get_true_child() const;
    const InverseNode<E>& get_false_child() const;
};

/**
 * True Dontcare combination
 */
template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_TX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;
    std::span<const E*> m_dontcare_elements;

    InverseNode<E> m_true_child;
    InverseNode<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_TX(const IInverseNode<E>* parent,
                                        SplitList useless_splits,
                                        formalism::GroundAtom<P> atom,
                                        std::span<const E*> true_elements,
                                        std::span<const E*> dontcare_elements);

    std::span<const E*> get_true_elements() const;
    std::span<const E*> get_dontcare_elements() const;

    InverseNode<E>& get_true_child();
    InverseNode<E>& get_dontcare_child();

    const InverseNode<E>& get_true_child() const;
    const InverseNode<E>& get_dontcare_child() const;
};

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_FX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_false_elements;
    std::span<const E*> m_dontcare_elements;

    InverseNode<E> m_false_child;
    InverseNode<E> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_FX(const IInverseNode<E>* parent,
                                        SplitList useless_splits,
                                        formalism::GroundAtom<P> atom,
                                        std::span<const E*> false_elements,
                                        std::span<const E*> dontcare_elements);

    std::span<const E*> get_false_elements() const;
    std::span<const E*> get_dontcare_elements() const;

    InverseNode<E>& get_false_child();
    InverseNode<E>& get_dontcare_child();

    const InverseNode<E>& get_false_child() const;
    const InverseNode<E>& get_dontcare_child() const;
};

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_T : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_true_elements;

    InverseNode<E> m_true_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_T(const IInverseNode<E>* parent,
                                       SplitList useless_splits,
                                       formalism::GroundAtom<P> atom,
                                       std::span<const E*> true_elements);

    std::span<const E*> get_true_elements() const;

    InverseNode<E>& get_true_child();

    const InverseNode<E>& get_true_child() const;
};

template<formalism::HasConjunctiveCondition E, formalism::IsFluentOrDerivedTag P>
class InverseAtomSelectorNode_F : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<E, P>, E, P>
{
private:
    // Candidates for further refinement.
    std::span<const E*> m_false_elements;

    InverseNode<E> m_false_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<E>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<E, P>, E, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<E, P>, E, P>::get_atom;

    explicit InverseAtomSelectorNode_F(const IInverseNode<E>* parent,
                                       SplitList useless_splits,
                                       formalism::GroundAtom<P> atom,
                                       std::span<const E*> false_elements);

    std::span<const E*> get_false_elements() const;

    InverseNode<E>& get_false_child();

    const InverseNode<E>& get_false_child() const;
};
}

#endif
