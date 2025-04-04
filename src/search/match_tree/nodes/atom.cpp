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

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TFX<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_TFX<E, P>::AtomSelectorNode_TFX(Node<E>&& true_child, Node<E>&& false_child, Node<E>&& dontcare_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TFX<E, P>, E, P>(atom),
    m_true_child(std::move(true_child)),
    m_false_child(std::move(false_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_false_child);
    assert(m_dontcare_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TFX<E, P>::generate_applicable_actions(const DenseState& state,
                                                             const ProblemImpl&,
                                                             std::vector<const INode<E>*>& ref_applicable_nodes,
                                                             std::vector<const E*>&) const
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TFX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TFX<E, P>::get_false_child() const
{
    return m_false_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TFX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_TFX<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_TFX<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>;

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TF<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_TF<E, P>::AtomSelectorNode_TF(Node<E>&& true_child, Node<E>&& false_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TF<E, P>, E, P>(atom),
    m_true_child(std::move(true_child)),
    m_false_child(std::move(false_child))
{
    assert(m_true_child);
    assert(m_false_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TF<E, P>::generate_applicable_actions(const DenseState& state,
                                                            const ProblemImpl&,
                                                            std::vector<const INode<E>*>& ref_applicable_nodes,
                                                            std::vector<const E*>&) const
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TF<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TF<E, P>::get_false_child() const
{
    return m_false_child;
}

template class AtomSelectorNode_TF<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_TF<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_TF<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>;

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TX<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_TX<E, P>::AtomSelectorNode_TX(Node<E>&& true_child, Node<E>&& dontcare_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_TX<E, P>, E, P>(atom),
    m_true_child(std::move(true_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_true_child);
    assert(m_dontcare_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_TX<E, P>::generate_applicable_actions(const DenseState& state,
                                                            const ProblemImpl&,
                                                            std::vector<const INode<E>*>& ref_applicable_nodes,
                                                            std::vector<const E*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_TX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_TX<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_TX<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_TX<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>;

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_FX<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_FX<E, P>::AtomSelectorNode_FX(Node<E>&& false_child, Node<E>&& dontcare_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_FX<E, P>, E, P>(atom),
    m_false_child(std::move(false_child)),
    m_dontcare_child(std::move(dontcare_child))
{
    assert(m_false_child);
    assert(m_dontcare_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_FX<E, P>::generate_applicable_actions(const DenseState& state,
                                                            const ProblemImpl&,
                                                            std::vector<const INode<E>*>& ref_applicable_nodes,
                                                            std::vector<const E*>&) const
{
    ref_applicable_nodes.push_back(m_dontcare_child.get());

    if (!state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_FX<E, P>::get_false_child() const
{
    return m_false_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_FX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class AtomSelectorNode_FX<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_FX<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_FX<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>;

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_T<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_T<E, P>::AtomSelectorNode_T(Node<E>&& true_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_T<E, P>, E, P>(atom),
    m_true_child(std::move(true_child))
{
    assert(m_true_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_T<E, P>::generate_applicable_actions(const DenseState& state,
                                                           const ProblemImpl&,
                                                           std::vector<const INode<E>*>& ref_applicable_nodes,
                                                           std::vector<const E*>&) const
{
    if (state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_true_child.get());
    }
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_T<E, P>::get_true_child() const
{
    return m_true_child;
};

template class AtomSelectorNode_T<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_T<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_T<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_T<GroundAxiomImpl, DerivedTag>;

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_F<E, P>::visit_impl(INodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
AtomSelectorNode_F<E, P>::AtomSelectorNode_F(Node<E>&& false_child, GroundAtom<P> atom) :
    AtomSelectorNodeBase<AtomSelectorNode_F<E, P>, E, P>(atom),
    m_false_child(std::move(false_child))
{
    assert(m_false_child);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void AtomSelectorNode_F<E, P>::generate_applicable_actions(const DenseState& state,
                                                           const ProblemImpl&,
                                                           std::vector<const INode<E>*>& ref_applicable_nodes,
                                                           std::vector<const E*>&) const
{
    if (!state.get_atoms<P>().get(this->m_atom->get_index()))
    {
        ref_applicable_nodes.push_back(m_false_child.get());
    }
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const Node<E>& AtomSelectorNode_F<E, P>::get_false_child() const
{
    return m_false_child;
}

template class AtomSelectorNode_F<GroundActionImpl, FluentTag>;
template class AtomSelectorNode_F<GroundActionImpl, DerivedTag>;
template class AtomSelectorNode_F<GroundAxiomImpl, FluentTag>;
template class AtomSelectorNode_F<GroundAxiomImpl, DerivedTag>;
}
