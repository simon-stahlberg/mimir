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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/languages/description_logics/equal_to.hpp"

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

/**
 * DL constructors
 */

/* Concepts */

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptBotImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptBotImpl> lhs,
                                                                                   loki::ObserverPtr<const mimir::dl::ConceptBotImpl> rhs) const
{
    return true;
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptTopImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptTopImpl> lhs,
                                                                                   loki::ObserverPtr<const mimir::dl::ConceptTopImpl> rhs) const
{
    return true;
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptAtomicGoalImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptIntersectionImpl> rhs) const
{
    return (lhs->get_concept_left() == rhs->get_concept_left()) && (lhs->get_concept_right() == rhs->get_concept_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> lhs,
                                                                                     loki::ObserverPtr<const mimir::dl::ConceptUnionImpl> rhs) const
{
    return (lhs->get_concept_left() == rhs->get_concept_left()) && (lhs->get_concept_right() == rhs->get_concept_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptNegationImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> lhs,
                                                                                        loki::ObserverPtr<const mimir::dl::ConceptNegationImpl> rhs) const
{
    return (lhs->get_concept() == rhs->get_concept());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptValueRestrictionImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role()) && (lhs->get_concept() == rhs->get_concept());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptExistentialQuantificationImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role()) && (lhs->get_concept() == rhs->get_concept());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapContainmentImpl> rhs) const
{
    return (lhs->get_role_left() == rhs->get_role_left()) && (lhs->get_role_right() == rhs->get_role_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> lhs,
    loki::ObserverPtr<const mimir::dl::ConceptRoleValueMapEqualityImpl> rhs) const
{
    return (lhs->get_role_left() == rhs->get_role_left()) && (lhs->get_role_right() == rhs->get_role_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::ConceptNominalImpl>>::operator()(loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> lhs,
                                                                                       loki::ObserverPtr<const mimir::dl::ConceptNominalImpl> rhs) const
{
    return (lhs->get_object() == rhs->get_object());
}

/* Roles */

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleUniversalImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> lhs,
                                                                                      loki::ObserverPtr<const mimir::dl::RoleUniversalImpl> rhs) const
{
    return true;
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> lhs,
                                                                                           loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>>>::operator()(loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> lhs,
                                                                                          loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::RoleAtomicGoalImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> lhs,
                                                                                         loki::ObserverPtr<const mimir::dl::RoleIntersectionImpl> rhs) const
{
    return (lhs->get_role_left() == rhs->get_role_left()) && (lhs->get_role_right() == rhs->get_role_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleUnionImpl> lhs,
                                                                                  loki::ObserverPtr<const mimir::dl::RoleUnionImpl> rhs) const
{
    return (lhs->get_role_left() == rhs->get_role_left()) && (lhs->get_role_right() == rhs->get_role_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleComplementImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleComplementImpl> lhs,
                                                                                       loki::ObserverPtr<const mimir::dl::RoleComplementImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleInverseImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleInverseImpl> lhs,
                                                                                    loki::ObserverPtr<const mimir::dl::RoleInverseImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleCompositionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> lhs,
                                                                                        loki::ObserverPtr<const mimir::dl::RoleCompositionImpl> rhs) const
{
    return (lhs->get_role_left() == rhs->get_role_left()) && (lhs->get_role_right() == rhs->get_role_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> lhs,
    loki::ObserverPtr<const mimir::dl::RoleTransitiveClosureImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> lhs,
    loki::ObserverPtr<const mimir::dl::RoleReflexiveTransitiveClosureImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> lhs,
                                                                                        loki::ObserverPtr<const mimir::dl::RoleRestrictionImpl> rhs) const
{
    return (lhs->get_role() == rhs->get_role()) && (lhs->get_concept() == rhs->get_concept());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::RoleIdentityImpl>>::operator()(loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> lhs,
                                                                                     loki::ObserverPtr<const mimir::dl::RoleIdentityImpl> rhs) const
{
    return (lhs->get_concept() == rhs->get_concept());
}

/**
 * DL grammar constructors
 */

template<mimir::dl::ConstructorTag D>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<D>> rhs) const
{
    return (lhs->get_non_terminal() == rhs->get_non_terminal()) && (lhs->get_constructor_or_non_terminals() == rhs->get_constructor_or_non_terminals());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<mimir::dl::Concept>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::DerivationRuleImpl<mimir::dl::Role>>>;

template<mimir::dl::ConstructorTag D>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<D>> rhs) const
{
    return (lhs->get_name() == rhs->get_name());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<mimir::dl::Concept>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::NonTerminalImpl<mimir::dl::Role>>>;

template<mimir::dl::ConstructorTag D>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<D>> rhs) const
{
    return (lhs->get_constructor_or_non_terminal() == rhs->get_constructor_or_non_terminal());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<mimir::dl::Concept>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConstructorOrNonTerminalImpl<mimir::dl::Role>>>;

/* Concepts */

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptBotImpl> rhs) const
{
    return true;
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptTopImpl> rhs) const
{
    return true;
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptAtomicGoalImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptIntersectionImpl> rhs) const
{
    return (lhs->get_concept_or_non_terminal_left() == rhs->get_concept_or_non_terminal_left())
           && (lhs->get_concept_or_non_terminal_right() == rhs->get_concept_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptUnionImpl> rhs) const
{
    return (lhs->get_concept_or_non_terminal_left() == rhs->get_concept_or_non_terminal_left())
           && (lhs->get_concept_or_non_terminal_right() == rhs->get_concept_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNegationImpl> rhs) const
{
    return (lhs->get_concept_or_non_terminal() == rhs->get_concept_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptValueRestrictionImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal()) && (lhs->get_concept_or_non_terminal() == rhs->get_concept_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptExistentialQuantificationImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal()) && (lhs->get_concept_or_non_terminal() == rhs->get_concept_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapContainmentImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal_left() == rhs->get_role_or_non_terminal_left())
           && (lhs->get_role_or_non_terminal_right() == rhs->get_role_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptRoleValueMapEqualityImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal_left() == rhs->get_role_or_non_terminal_left())
           && (lhs->get_role_or_non_terminal_right() == rhs->get_role_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::ConceptNominalImpl> rhs) const
{
    return (lhs->get_object() == rhs->get_object());
}

/* Roles */

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleUniversalImpl> rhs) const
{
    return true;
}

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<P>> rhs) const
{
    return (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicStateImpl<mimir::Derived>>>;

template<mimir::PredicateTag P>
bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<P>> rhs) const
{
    return (lhs->is_negated() == rhs->is_negated()) && (lhs->get_predicate() == rhs->get_predicate());
}

template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Static>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Fluent>>>;
template struct std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleAtomicGoalImpl<mimir::Derived>>>;

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleIntersectionImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal_left() == rhs->get_role_or_non_terminal_left())
           && (lhs->get_role_or_non_terminal_right() == rhs->get_role_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl>>::operator()(loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> lhs,
                                                                                           loki::ObserverPtr<const mimir::dl::grammar::RoleUnionImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal_left() == rhs->get_role_or_non_terminal_left())
           && (lhs->get_role_or_non_terminal_right() == rhs->get_role_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleComplementImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleInverseImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleCompositionImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal_left() == rhs->get_role_or_non_terminal_left())
           && (lhs->get_role_or_non_terminal_right() == rhs->get_role_or_non_terminal_right());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleTransitiveClosureImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleReflexiveTransitiveClosureImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleRestrictionImpl> rhs) const
{
    return (lhs->get_role_or_non_terminal() == rhs->get_role_or_non_terminal()) && (lhs->get_concept_or_non_terminal() == rhs->get_concept_or_non_terminal());
}

bool std::equal_to<loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl>>::operator()(
    loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> lhs,
    loki::ObserverPtr<const mimir::dl::grammar::RoleIdentityImpl> rhs) const
{
    return (lhs->get_concept_or_non_terminal() == rhs->get_concept_or_non_terminal());
}
