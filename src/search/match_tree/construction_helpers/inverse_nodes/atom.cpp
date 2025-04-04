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

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

/**
 * True False Dontcare combination
 */

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_TFX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TFX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_false_child()
{
    return m_false_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_false_child() const
{
    return m_false_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TFX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_TFX<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_TFX<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_TFX<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_TFX<GroundAxiomImpl, DerivedTag>;

/**
 * True False combination
 */
template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_TF<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TF<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TF<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_false_child()
{
    return m_false_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TF<E, P>::get_false_child() const
{
    return m_false_child;
}

template class InverseAtomSelectorNode_TF<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_TF<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_TF<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_TF<GroundAxiomImpl, DerivedTag>;

/**
 * True Dontcare combination
 */
template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_TX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TX<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_TX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_true_child() const
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_TX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_TX<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_TX<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_TX<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_TX<GroundAxiomImpl, DerivedTag>;

/**
 * False Dontcare combination
 */
template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_FX<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_FX<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_FX<E, P>::get_dontcare_elements() const
{
    return m_dontcare_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_false_child()
{
    return m_false_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_dontcare_child()
{
    return m_dontcare_child;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_false_child() const
{
    return m_false_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_FX<E, P>::get_dontcare_child() const
{
    return m_dontcare_child;
}

template class InverseAtomSelectorNode_FX<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_FX<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_FX<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_FX<GroundAxiomImpl, DerivedTag>;

/**
 * True combination
 */
template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_T<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_T<E, P>::get_true_elements() const
{
    return m_true_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_T<E, P>::get_true_child()
{
    return m_true_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_T<E, P>::get_true_child() const
{
    return m_true_child;
};

template class InverseAtomSelectorNode_T<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_T<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_T<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_T<GroundAxiomImpl, DerivedTag>;

/**
 * False combination
 */
template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
void InverseAtomSelectorNode_F<E, P>::visit_impl(IInverseNodeVisitor<E>& visitor) const
{
    visitor.accept(*this);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
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

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
std::span<const E*> InverseAtomSelectorNode_F<E, P>::get_false_elements() const
{
    return m_false_elements;
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
InverseNode<E>& InverseAtomSelectorNode_F<E, P>::get_false_child()
{
    return m_false_child;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
const InverseNode<E>& InverseAtomSelectorNode_F<E, P>::get_false_child() const
{
    return m_false_child;
};

template class InverseAtomSelectorNode_F<GroundActionImpl, FluentTag>;
template class InverseAtomSelectorNode_F<GroundActionImpl, DerivedTag>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, FluentTag>;
template class InverseAtomSelectorNode_F<GroundAxiomImpl, DerivedTag>;

}
