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

namespace mimir::search::match_tree
{

/**
 * True False Dontcare combination
 */

template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_TFX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_TFX<E, P>::InverseAtomSelectorNode_TFX(const IInverseNode<E>* parent,
                                                               SplitList useless_splits,
                                                               GroundAtom<P> atom,
                                                               std::span<const E*> true_elements,
                                                               std::span<const E*> false_elements,
                                                               std::span<const E*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TFX<E, P>, E, P>(parent, std::move(useless_splits), atom),
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

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_false_child()
{
    return m_false_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_false_child() const
{
    return m_false_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_dontcare_child() const
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
template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_TF<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_TF<E, P>::InverseAtomSelectorNode_TF(const IInverseNode<E>* parent,
                                                             SplitList useless_splits,
                                                             GroundAtom<P> atom,
                                                             std::span<const E*> true_elements,
                                                             std::span<const E*> false_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TF<E, P>, E, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_false_elements(false_elements),
    m_true_child(nullptr),
    m_false_child(nullptr)
{
    assert(!m_true_elements.empty());
    assert(!m_false_elements.empty());
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TF<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TF<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_false_child()
{
    return m_false_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_false_child() const
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
template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_TX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_TX<E, P>::InverseAtomSelectorNode_TX(const IInverseNode<E>* parent,
                                                             SplitList useless_splits,
                                                             GroundAtom<P> atom,
                                                             std::span<const E*> true_elements,
                                                             std::span<const E*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_TX<E, P>, E, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_dontcare_elements(dontcare_elements),
    m_true_child(nullptr),
    m_dontcare_child(nullptr)
{
    assert(!m_true_elements.empty());
    assert(!m_dontcare_elements.empty());
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TX<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_TX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_dontcare_child() const
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
template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_FX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_FX<E, P>::InverseAtomSelectorNode_FX(const IInverseNode<E>* parent,
                                                             SplitList useless_splits,
                                                             GroundAtom<P> atom,
                                                             std::span<const E*> false_elements,
                                                             std::span<const E*> dontcare_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_FX<E, P>, E, P>(parent, std::move(useless_splits), atom),
    m_false_elements(false_elements),
    m_dontcare_elements(dontcare_elements),
    m_false_child(nullptr),
    m_dontcare_child(nullptr)
{
    assert(!m_false_elements.empty());
    assert(!m_dontcare_elements.empty());
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_FX<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_FX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_false_child()
{
    return m_false_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_false_child() const
{
    return m_false_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_dontcare_child() const
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
template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_T<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_T<E, P>::InverseAtomSelectorNode_T(const IInverseNode<E>* parent,
                                                           SplitList useless_splits,
                                                           GroundAtom<P> atom,
                                                           std::span<const E*> true_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_T<E, P>, E, P>(parent, std::move(useless_splits), atom),
    m_true_elements(true_elements),
    m_true_child(nullptr)
{
    assert(!m_true_elements.empty());
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_T<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_T<E, P>::get_true_child()
{
    return m_true_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_T<E, P>::get_true_child() const
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
template<HasConjunctiveCondition E, FluentOrDerived P>
void InverseAtomSelectorNode_F<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseAtomSelectorNode_F<E, P>::InverseAtomSelectorNode_F(const IInverseNode<E>* parent,
                                                           SplitList useless_splits,
                                                           GroundAtom<P> atom,
                                                           std::span<const E*> false_elements) :
    InverseAtomSelectorNodeBase<InverseAtomSelectorNode_F<E, P>, E, P>(parent, std::move(useless_splits), atom),
    m_false_elements(false_elements),
    m_false_child(nullptr)
{
    assert(!m_false_elements.empty());
}

template<HasConjunctiveCondition E, FluentOrDerived P>
std::span<const E*> InverseAtomSelectorNode_F<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<HasConjunctiveCondition E, FluentOrDerived P>
InverseNode<E>& InverseAtomSelectorNode_F<E, P>::get_false_child()
{
    return m_false_child;
};

template<HasConjunctiveCondition E, FluentOrDerived P>
const InverseNode<E>& InverseAtomSelectorNode_F<E, P>::get_false_child() const
{
    return m_false_child;
};

template class InverseAtomSelectorNode_F<GroundActionImpl, Fluent>;
template class InverseAtomSelectorNode_F<GroundActionImpl, Derived>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, Fluent>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, Derived>;

}
