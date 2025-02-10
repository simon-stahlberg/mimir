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
};

/**
 * True False Dontcare combination
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TFX : public AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>
{
private:
    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    explicit AtomSelectorNode_TFX(Node<Element>&& true_succ, Node<Element>&& false_succ, Node<Element>&& dontcare_succ, GroundAtom<P> atom) :
        AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>(atom),
        m_true_succ(std::move(true_succ)),
        m_false_succ(std::move(false_succ)),
        m_dontcare_succ(std::move(dontcare_succ))
    {
        assert(m_true_succ);
        assert(m_false_succ);
        assert(m_dontcare_succ);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        ref_applicable_nodes.push_back(m_dontcare_succ.get());

        if (state.get_atoms<P>().get(this->m_atom->get_index()))
        {
            ref_applicable_nodes.push_back(m_true_succ.get());
        }
        else
        {
            ref_applicable_nodes.push_back(m_false_succ.get());
        }
    }
};

/**
 * True False combination.
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TF : public AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>
{
private:
    Node<Element> m_true_succ;
    Node<Element> m_false_succ;

public:
    explicit AtomSelectorNode_TF(Node<Element>&& true_succ, Node<Element>&& false_succ, GroundAtom<P> atom) :
        AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>(atom),
        m_true_succ(std::move(true_succ)),
        m_false_succ(std::move(false_succ))
    {
        assert(m_true_succ);
        assert(m_false_succ);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        if (state.get_atoms<P>().get(this->m_atom->get_index()))
        {
            ref_applicable_nodes.push_back(m_true_succ.get());
        }
        else
        {
            ref_applicable_nodes.push_back(m_false_succ.get());
        }
    }
};

/**
 * True Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_TX : public AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>
{
private:
    Node<Element> m_true_succ;
    Node<Element> m_dontcare_succ;

public:
    explicit AtomSelectorNode_TX(Node<Element>&& true_succ, Node<Element>&& dontcare_succ, GroundAtom<P> atom) :
        AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>(atom),
        m_true_succ(std::move(true_succ)),
        m_dontcare_succ(std::move(dontcare_succ))
    {
        assert(m_true_succ);
        assert(m_dontcare_succ);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        ref_applicable_nodes.push_back(m_dontcare_succ.get());

        if (state.get_atoms<P>().get(this->m_atom->get_index()))
            ref_applicable_nodes.push_back(m_true_succ.get());
    }
};

/**
 * False Dontcare combination.
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode_FX : public AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>
{
private:
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    explicit AtomSelectorNode_FX(Node<Element>&& false_succ, Node<Element>&& dontcare_succ, GroundAtom<P> atom) :
        AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>(atom),
        m_false_succ(std::move(false_succ)),
        m_dontcare_succ(std::move(dontcare_succ))
    {
        assert(m_false_succ);
        assert(m_dontcare_succ);
    }

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        ref_applicable_nodes.push_back(m_dontcare_succ.get());

        if (!state.get_atoms<P>().get(this->m_atom->get_index()))
            ref_applicable_nodes.push_back(m_false_succ.get());
    }
};
}

#endif
