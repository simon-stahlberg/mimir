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

namespace mimir::match_tree
{
template<typename Derived_, HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNodeBase : public IInverseNode<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    GroundAtom<P> m_atom;

public:
    explicit InverseAtomSelectorNodeBase(const IInverseNode<Element>* parent, SplitList useless_splits, size_t root_distance, GroundAtom<P> atom) :
        IInverseNode<Element>(parent, std::move(useless_splits), root_distance),
        m_atom(atom)
    {
        assert(m_atom);
    }
    InverseAtomSelectorNodeBase(const InverseAtomSelectorNodeBase& other) = delete;
    InverseAtomSelectorNodeBase& operator=(const InverseAtomSelectorNodeBase& other) = delete;
    InverseAtomSelectorNodeBase(InverseAtomSelectorNodeBase&& other) = delete;
    InverseAtomSelectorNodeBase& operator=(InverseAtomSelectorNodeBase&& other) = delete;

    void visit(IInverseNodeVisitor<Element>& visitor) const override { self().visit_impl(visitor); }

    GroundAtom<P> get_atom() const { return m_atom; };
};

/**
 * True False Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TFX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_false_elements;
    std::span<const Element*> m_dontcare_elements;

    InverseNode<Element> m_true_child;
    InverseNode<Element> m_false_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TFX(const IInverseNode<Element>* parent,
                                         SplitList useless_splits,
                                         size_t root_distance,
                                         GroundAtom<P> atom,
                                         std::span<const Element*> true_elements,
                                         std::span<const Element*> false_elements,
                                         std::span<const Element*> dontcare_elements);

    std::span<const Element*> get_true_elements() const;
    std::span<const Element*> get_false_elements() const;
    std::span<const Element*> get_dontcare_elements() const;

    InverseNode<Element>& get_true_child();
    InverseNode<Element>& get_false_child();
    InverseNode<Element>& get_dontcare_child();

    const InverseNode<Element>& get_true_child() const;
    const InverseNode<Element>& get_false_child() const;
    const InverseNode<Element>& get_dontcare_child() const;
};

/**
 * True False combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TF : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_false_elements;

    InverseNode<Element> m_true_child;
    InverseNode<Element> m_false_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TF(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> true_elements,
                                        std::span<const Element*> false_elements);

    std::span<const Element*> get_true_elements() const;
    std::span<const Element*> get_false_elements() const;

    InverseNode<Element>& get_true_child();
    InverseNode<Element>& get_false_child();

    const InverseNode<Element>& get_true_child() const;
    const InverseNode<Element>& get_false_child() const;
};

/**
 * True Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_dontcare_elements;

    InverseNode<Element> m_true_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TX(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> true_elements,
                                        std::span<const Element*> dontcare_elements);

    std::span<const Element*> get_true_elements() const;
    std::span<const Element*> get_dontcare_elements() const;

    InverseNode<Element>& get_true_child();
    InverseNode<Element>& get_dontcare_child();

    const InverseNode<Element>& get_true_child() const;
    const InverseNode<Element>& get_dontcare_child() const;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_FX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_false_elements;
    std::span<const Element*> m_dontcare_elements;

    InverseNode<Element> m_false_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_FX(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> false_elements,
                                        std::span<const Element*> dontcare_elements);

    std::span<const Element*> get_false_elements() const;
    std::span<const Element*> get_dontcare_elements() const;

    InverseNode<Element>& get_false_child();
    InverseNode<Element>& get_dontcare_child();

    const InverseNode<Element>& get_false_child() const;
    const InverseNode<Element>& get_dontcare_child() const;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_T : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;

    InverseNode<Element> m_true_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_T(const IInverseNode<Element>* parent,
                                       SplitList useless_splits,
                                       size_t root_distance,
                                       GroundAtom<P> atom,
                                       std::span<const Element*> true_elements);

    std::span<const Element*> get_true_elements() const;

    InverseNode<Element>& get_true_child();

    const InverseNode<Element>& get_true_child() const;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_F : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<Element, P>, Element, P>
{
private:
    // Candidates for further refinement.
    std::span<const Element*> m_false_elements;

    InverseNode<Element> m_false_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const;

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_F(const IInverseNode<Element>* parent,
                                       SplitList useless_splits,
                                       size_t root_distance,
                                       GroundAtom<P> atom,
                                       std::span<const Element*> false_elements);

    std::span<const Element*> get_false_elements() const;

    InverseNode<Element>& get_false_child();

    const InverseNode<Element>& get_false_child() const;
};
}

#endif
