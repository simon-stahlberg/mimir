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

#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"

namespace mimir::match_tree
{

/**
 * True False Dontcare combination
 */

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_TFX<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_TFX<Element, P>::InverseAtomSelectorNode_TFX(const IInverseNode<Element>* parent,
                                                                     SplitList useless_splits,
                                                                     GroundAtom<P> atom,
                                                                     std::span<const Element*> true_elements,
                                                                     std::span<const Element*> false_elements,
                                                                     std::span<const Element*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
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

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TFX<Element, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TFX<Element, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TFX<Element, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_false_child()
{
    return m_false_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_false_child() const
{
    return m_false_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TFX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_TFX<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_TFX<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_TFX<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_TFX<GroundAxiomImpl, Derived>;

/**
 * True False combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_TF<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_TF<Element, P>::InverseAtomSelectorNode_TF(const IInverseNode<Element>* parent,
                                                                   SplitList useless_splits,
                                                                   GroundAtom<P> atom,
                                                                   std::span<const Element*> true_elements,
                                                                   std::span<const Element*> false_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_false_elements(false_elements),
    m_true_child(nullptr),
    m_false_child(nullptr)
{
    assert(!m_true_elements.empty());
    assert(!m_false_elements.empty());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TF<Element, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TF<Element, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TF<Element, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TF<Element, P>::get_false_child()
{
    return m_false_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TF<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TF<Element, P>::get_false_child() const
{
    return m_false_child;
}

template class InverseAtomSelectorNode_TF<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_TF<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_TF<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_TF<GroundAxiomImpl, Derived>;

/**
 * True Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_TX<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_TX<Element, P>::InverseAtomSelectorNode_TX(const IInverseNode<Element>* parent,
                                                                   SplitList useless_splits,
                                                                   GroundAtom<P> atom,
                                                                   std::span<const Element*> true_elements,
                                                                   std::span<const Element*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_dontcare_elements(dontcare_elements),
    m_true_child(nullptr),
    m_dontcare_child(nullptr)
{
    assert(!m_true_elements.empty());
    assert(!m_dontcare_elements.empty());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TX<Element, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_TX<Element, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TX<Element, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_TX<Element, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TX<Element, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_TX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_TX<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_TX<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_TX<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_TX<GroundAxiomImpl, Derived>;

/**
 * False Dontcare combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_FX<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_FX<Element, P>::InverseAtomSelectorNode_FX(const IInverseNode<Element>* parent,
                                                                   SplitList useless_splits,
                                                                   GroundAtom<P> atom,
                                                                   std::span<const Element*> false_elements,
                                                                   std::span<const Element*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
    m_false_elements(false_elements),
    m_dontcare_elements(dontcare_elements),
    m_false_child(nullptr),
    m_dontcare_child(nullptr)
{
    assert(!m_false_elements.empty());
    assert(!m_dontcare_elements.empty());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_FX<Element, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_FX<Element, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_FX<Element, P>::get_false_child()
{
    return m_false_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_FX<Element, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_FX<Element, P>::get_false_child() const
{
    return m_false_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_FX<Element, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_FX<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_FX<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_FX<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_FX<GroundAxiomImpl, Derived>;

/**
 * True combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_T<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_T<Element, P>::InverseAtomSelectorNode_T(const IInverseNode<Element>* parent,
                                                                 SplitList useless_splits,
                                                                 GroundAtom<P> atom,
                                                                 std::span<const Element*> true_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_true_child(nullptr)
{
    assert(!m_true_elements.empty());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_T<Element, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_T<Element, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_T<Element, P>::get_true_child() const
{
    return m_true_child;
};

template class InverseAtomSelectorNode_T<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_T<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_T<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_T<GroundAxiomImpl, Derived>;

/**
 * False combination
 */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
void InverseAtomSelectorNode_F<Element, P>::visit_impl(IInverseNodeVisitor<Element>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseAtomSelectorNode_F<Element, P>::InverseAtomSelectorNode_F(const IInverseNode<Element>* parent,
                                                                 SplitList useless_splits,
                                                                 GroundAtom<P> atom,
                                                                 std::span<const Element*> false_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<Element, P>, Element, P>(parent, std::move(useless_splits), atom),
    m_false_elements(false_elements),
    m_false_child(nullptr)
{
    assert(!m_false_elements.empty());
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
std::span<const Element*> InverseAtomSelectorNode_F<Element, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
InverseNode<Element>& InverseAtomSelectorNode_F<Element, P>::get_false_child()
{
    return m_false_child;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
const InverseNode<Element>& InverseAtomSelectorNode_F<Element, P>::get_false_child() const
{
    return m_false_child;
};

template class InverseAtomSelectorNode_F<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_F<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, Derived>;

}
