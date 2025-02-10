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

#include "mimir/formalism/ground_atom.hpp"
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
    // Meta data
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
    // Meta data
    GroundAtom<P> m_atom;

    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_false_elements;
    std::span<const Element*> m_dontcare_elements;

    // Wrapped into unique_ptr to ensure no invalidaiton of the node after move.
    InverseNode<Element> m_true_child;
    InverseNode<Element> m_false_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TFX(const IInverseNode<Element>* parent,
                                         SplitList useless_splits,
                                         size_t root_distance,
                                         GroundAtom<P> atom,
                                         std::span<const Element*> true_elements,
                                         std::span<const Element*> false_elements,
                                         std::span<const Element*> dontcare_elements) :
        InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>(parent, std::move(useless_splits), root_distance, atom),
        m_true_elements(true_elements),
        m_false_elements(false_elements),
        m_dontcare_elements(dontcare_elements),
        m_true_child(nullptr),
        m_false_child(nullptr),
        m_dontcare_child(nullptr)
    {
        assert(!m_true_elements.empty());
        assert(!m_false_elements.empty());
        assert(!m_dontcare_elements.empty());
    }
    InverseAtomSelectorNode_TFX(const InverseAtomSelectorNode_TFX& other) = delete;
    InverseAtomSelectorNode_TFX& operator=(const InverseAtomSelectorNode_TFX& other) = delete;
    InverseAtomSelectorNode_TFX(InverseAtomSelectorNode_TFX&& other) = delete;
    InverseAtomSelectorNode_TFX& operator=(InverseAtomSelectorNode_TFX&& other) = delete;

    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_false_elements() const { return m_false_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }

    InverseNode<Element>& get_true_child() { return m_true_child; };
    InverseNode<Element>& get_false_child() { return m_false_child; }
    InverseNode<Element>& get_dontcare_child() { return m_dontcare_child; }

    const InverseNode<Element>& get_true_child() const { return m_true_child; };
    const InverseNode<Element>& get_false_child() const { return m_false_child; }
    const InverseNode<Element>& get_dontcare_child() const { return m_dontcare_child; }
};

/**
 * True False combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TF : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>
{
private:
    // Meta data
    GroundAtom<P> m_atom;

    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_false_elements;

    // Wrapped into unique_ptr to ensure no invalidaiton of the node after move.
    InverseNode<Element> m_true_child;
    InverseNode<Element> m_false_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TF(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> true_elements,
                                        std::span<const Element*> false_elements) :
        InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>(parent, std::move(useless_splits), root_distance, atom),
        m_true_elements(true_elements),
        m_false_elements(false_elements),
        m_true_child(nullptr),
        m_false_child(nullptr)
    {
        assert(!m_true_elements.empty());
        assert(!m_false_elements.empty());
    }
    InverseAtomSelectorNode_TF(const InverseAtomSelectorNode_TF& other) = delete;
    InverseAtomSelectorNode_TF& operator=(const InverseAtomSelectorNode_TF& other) = delete;
    InverseAtomSelectorNode_TF(InverseAtomSelectorNode_TF&& other) = delete;
    InverseAtomSelectorNode_TF& operator=(InverseAtomSelectorNode_TF&& other) = delete;

    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_false_elements() const { return m_false_elements; }

    InverseNode<Element>& get_true_child() { return m_true_child; };
    InverseNode<Element>& get_false_child() { return m_false_child; }

    const InverseNode<Element>& get_true_child() const { return m_true_child; };
    const InverseNode<Element>& get_false_child() const { return m_false_child; }
};

/**
 * True Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_TX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>
{
private:
    // Meta data
    GroundAtom<P> m_atom;

    // Candidates for further refinement.
    std::span<const Element*> m_true_elements;
    std::span<const Element*> m_dontcare_elements;

    // Wrapped into unique_ptr to ensure no invalidaiton of the node after move.
    InverseNode<Element> m_true_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_TX(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> true_elements,
                                        std::span<const Element*> dontcare_elements) :
        InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>(parent, std::move(useless_splits), root_distance, atom),
        m_true_elements(true_elements),
        m_dontcare_elements(dontcare_elements),
        m_true_child(nullptr),
        m_dontcare_child(nullptr)
    {
        assert(!m_true_elements.empty());
        assert(!m_dontcare_elements.empty());
    }
    InverseAtomSelectorNode_TX(const InverseAtomSelectorNode_TX& other) = delete;
    InverseAtomSelectorNode_TX& operator=(const InverseAtomSelectorNode_TX& other) = delete;
    InverseAtomSelectorNode_TX(InverseAtomSelectorNode_TX&& other) = delete;
    InverseAtomSelectorNode_TX& operator=(InverseAtomSelectorNode_TX&& other) = delete;

    std::span<const Element*> get_true_elements() const { return m_true_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }

    InverseNode<Element>& get_true_child() { return m_true_child; };
    InverseNode<Element>& get_dontcare_child() { return m_dontcare_child; }

    const InverseNode<Element>& get_true_child() const { return m_true_child; };
    const InverseNode<Element>& get_dontcare_child() const { return m_dontcare_child; }
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class InverseAtomSelectorNode_FX : public InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>
{
private:
    // Meta data
    GroundAtom<P> m_atom;

    // Candidates for further refinement.
    std::span<const Element*> m_false_elements;
    std::span<const Element*> m_dontcare_elements;

    // Wrapped into unique_ptr to ensure no invalidaiton of the node after move.
    InverseNode<Element> m_false_child;
    InverseNode<Element> m_dontcare_child;

    /* Implement interface*/

    void visit_impl(IInverseNodeVisitor<Element>& visitor) const { visitor.accept(*this); }

    friend class InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>;

public:
    using InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>::get_atom;

    explicit InverseAtomSelectorNode_FX(const IInverseNode<Element>* parent,
                                        SplitList useless_splits,
                                        size_t root_distance,
                                        GroundAtom<P> atom,
                                        std::span<const Element*> false_elements,
                                        std::span<const Element*> dontcare_elements) :
        InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>(parent, std::move(useless_splits), root_distance, atom),
        m_false_elements(false_elements),
        m_dontcare_elements(dontcare_elements),
        m_false_child(nullptr),
        m_dontcare_child(nullptr)
    {
        assert(!m_false_elements.empty());
        assert(!m_dontcare_elements.empty());
    }
    InverseAtomSelectorNode_FX(const InverseAtomSelectorNode_FX& other) = delete;
    InverseAtomSelectorNode_FX& operator=(const InverseAtomSelectorNode_FX& other) = delete;
    InverseAtomSelectorNode_FX(InverseAtomSelectorNode_FX&& other) = delete;
    InverseAtomSelectorNode_FX& operator=(InverseAtomSelectorNode_FX&& other) = delete;

    std::span<const Element*> get_false_elements() const { return m_false_elements; }
    std::span<const Element*> get_dontcare_elements() const { return m_dontcare_elements; }

    InverseNode<Element>& get_false_child() { return m_false_child; };
    InverseNode<Element>& get_dontcare_child() { return m_dontcare_child; }

    const InverseNode<Element>& get_false_child() const { return m_false_child; };
    const InverseNode<Element>& get_dontcare_child() const { return m_dontcare_child; }
};
}

#endif
