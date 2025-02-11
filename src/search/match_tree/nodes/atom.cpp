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

#include "mimir/search/match_tree/nodes/atom.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/dense_state.hpp"

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TFX<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_TFX<Element, P>::AtomSelectorNode_TFX(Node<Element>&& true_child,
                                                       Node<Element>&& false_child,
                                                       Node<Element>&& dontcare_child,
                                                       GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TFX<Element, P>, Element, P>(atom),
    m_true_child(std::move(true_child)),
    m_false_child(std::move(false_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_false_child);
    assert(m_dontcare_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TFX<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                   std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                   std::vector<const Element*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
    else
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TFX<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TFX<Element, P>::get_false_child() const
{
    return m_false_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TFX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_TFX<GroundActionImpl, Fluent>;
template class AtomSelectorNode_TFX<GroundActionImpl, Derived>;
template class AtomSelectorNode_TFX<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_TFX<GroundAxiomImpl, Derived>;

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TF<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_TF<Element, P>::AtomSelectorNode_TF(Node<Element>&& true_child, Node<Element>&& false_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TF<Element, P>, Element, P>(atom),
    m_true_child(std::move(true_child)),
    m_false_child(std::move(false_child))
{
    assert(m_true_child);
    assert(m_false_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TF<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                  std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                  std::vector<const Element*>&) const
{
    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
    else
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TF<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TF<Element, P>::get_false_child() const
{
    return m_false_child;
}

template class AtomSelectorNode_TF<GroundActionImpl, Fluent>;
template class AtomSelectorNode_TF<GroundActionImpl, Derived>;
template class AtomSelectorNode_TF<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_TF<GroundAxiomImpl, Derived>;

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TX<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_TX<Element, P>::AtomSelectorNode_TX(Node<Element>&& true_child, Node<Element>&& dontcare_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TX<Element, P>, Element, P>(atom),
    m_true_child(std::move(true_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_dontcare_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_TX<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                  std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                  std::vector<const Element*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TX<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_TX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_TX<GroundActionImpl, Fluent>;
template class AtomSelectorNode_TX<GroundActionImpl, Derived>;
template class AtomSelectorNode_TX<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_TX<GroundAxiomImpl, Derived>;

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_FX<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_FX<Element, P>::AtomSelectorNode_FX(Node<Element>&& false_child, Node<Element>&& dontcare_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_FX<Element, P>, Element, P>(atom),
    m_false_child(std::move(false_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_false_child);
    assert(m_dontcare_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_FX<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                  std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                  std::vector<const Element*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (!state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_FX<Element, P>::get_false_child() const
{
    return m_false_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_FX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_FX<GroundActionImpl, Fluent>;
template class AtomSelectorNode_FX<GroundActionImpl, Derived>;
template class AtomSelectorNode_FX<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_FX<GroundAxiomImpl, Derived>;

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_T<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_T<Element, P>::AtomSelectorNode_T(Node<Element>&& true_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_T<Element, P>, Element, P>(atom),
    m_true_child(std::move(true_child))
{
    assert(m_true_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_T<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                 std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                 std::vector<const Element*>&) const
{
    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_T<Element, P>::get_true_child() const
{
    return m_true_child;
};

template class AtomSelectorNode_T<GroundActionImpl, Fluent>;
template class AtomSelectorNode_T<GroundActionImpl, Derived>;
template class AtomSelectorNode_T<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_T<GroundAxiomImpl, Derived>;

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_F<Element, P>::visit_impl(INodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
AtomSelectorNode_F<Element, P>::AtomSelectorNode_F(Node<Element>&& false_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_F<Element, P>, Element, P>(atom),
    m_false_child(std::move(false_child))
{
    assert(m_false_child);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void AtomSelectorNode_F<Element, P>::generate_applicable_actions(const DenseState& state,
                                                                 std::vector<const INode<Element>*>& ref_applicable_nodes,
                                                                 std::vector<const Element*>&) const
{
    if (!state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const Node<Element>& AtomSelectorNode_F<Element, P>::get_false_child() const
{
    return m_false_child;
}

template class AtomSelectorNode_F<GroundActionImpl, Fluent>;
template class AtomSelectorNode_F<GroundActionImpl, Derived>;
template class AtomSelectorNode_F<GroundAxiomImpl, Fluent>;
template class AtomSelectorNode_F<GroundAxiomImpl, Derived>;
}
