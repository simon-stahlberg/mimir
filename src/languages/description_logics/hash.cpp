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

#include "mimir/languages/description_logics/hash.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

namespace mimir::dl
{

/**
 * DL constructors
 */

/* Concepts */

size_t UniqueDLHasher<const ConceptBotImpl*>::operator()(const ConceptBotImpl* e) const { return mimir::hash_combine(e); }

size_t UniqueDLHasher<const ConceptTopImpl*>::operator()(const ConceptTopImpl* e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<const ConceptAtomicStateImpl<P>*>::operator()(const ConceptAtomicStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const ConceptAtomicStateImpl<Static>*>::operator()(const ConceptAtomicStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const ConceptAtomicStateImpl<Fluent>*>::operator()(const ConceptAtomicStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const ConceptAtomicStateImpl<Derived>*>::operator()(const ConceptAtomicStateImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>::operator()(const ConceptAtomicGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<const ConceptAtomicGoalImpl<Static>*>::operator()(const ConceptAtomicGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const ConceptAtomicGoalImpl<Fluent>*>::operator()(const ConceptAtomicGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const ConceptAtomicGoalImpl<Derived>*>::operator()(const ConceptAtomicGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const ConceptIntersectionImpl*>::operator()(const ConceptIntersectionImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

size_t UniqueDLHasher<const ConceptUnionImpl*>::operator()(const ConceptUnionImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

size_t UniqueDLHasher<const ConceptNegationImpl*>::operator()(const ConceptNegationImpl* e) const { return mimir::hash_combine(e->get_concept()); }

size_t UniqueDLHasher<const ConceptValueRestrictionImpl*>::operator()(const ConceptValueRestrictionImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const ConceptExistentialQuantificationImpl*>::operator()(const ConceptExistentialQuantificationImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>::operator()(const ConceptRoleValueMapContainmentImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>::operator()(const ConceptRoleValueMapEqualityImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const ConceptNominalImpl*>::operator()(const ConceptNominalImpl* e) const { return mimir::hash_combine(e->get_object()); }

/* Roles */

size_t UniqueDLHasher<const RoleUniversalImpl*>::operator()(const RoleUniversalImpl* e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<const RoleAtomicStateImpl<P>*>::operator()(const RoleAtomicStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const RoleAtomicStateImpl<Static>*>::operator()(const RoleAtomicStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const RoleAtomicStateImpl<Fluent>*>::operator()(const RoleAtomicStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const RoleAtomicStateImpl<Derived>*>::operator()(const RoleAtomicStateImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniqueDLHasher<const RoleAtomicGoalImpl<P>*>::operator()(const RoleAtomicGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<const RoleAtomicGoalImpl<Static>*>::operator()(const RoleAtomicGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const RoleAtomicGoalImpl<Fluent>*>::operator()(const RoleAtomicGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const RoleAtomicGoalImpl<Derived>*>::operator()(const RoleAtomicGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const RoleIntersectionImpl*>::operator()(const RoleIntersectionImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const RoleUnionImpl*>::operator()(const RoleUnionImpl* e) const { return mimir::hash_combine(e->get_role_left(), e->get_role_right()); }

size_t UniqueDLHasher<const RoleComplementImpl*>::operator()(const RoleComplementImpl* e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<const RoleInverseImpl*>::operator()(const RoleInverseImpl* e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<const RoleCompositionImpl*>::operator()(const RoleCompositionImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const RoleTransitiveClosureImpl*>::operator()(const RoleTransitiveClosureImpl* e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>::operator()(const RoleReflexiveTransitiveClosureImpl* e) const
{
    return mimir::hash_combine(e->get_role());
}

size_t UniqueDLHasher<const RoleRestrictionImpl*>::operator()(const RoleRestrictionImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const RoleIdentityImpl*>::operator()(const RoleIdentityImpl* e) const { return mimir::hash_combine(e->get_concept()); }

/**
 * DL grammar constructors
 */

template<ConstructorTag D>
size_t UniqueDLHasher<const grammar::DerivationRuleImpl<D>*>::operator()(const grammar::DerivationRuleImpl<D>* e) const
{
    return mimir::hash_combine(e->get_choices());
}

template size_t UniqueDLHasher<const grammar::DerivationRuleImpl<Concept>*>::operator()(const grammar::DerivationRuleImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::DerivationRuleImpl<Role>*>::operator()(const grammar::DerivationRuleImpl<Role>* e) const;

template<ConstructorTag D>
size_t UniqueDLHasher<const grammar::NonTerminalImpl<D>*>::operator()(const grammar::NonTerminalImpl<D>* e) const
{
    return mimir::hash_combine(e->get_name());
}

template size_t UniqueDLHasher<const grammar::NonTerminalImpl<Concept>*>::operator()(const grammar::NonTerminalImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::NonTerminalImpl<Role>*>::operator()(const grammar::NonTerminalImpl<Role>* e) const;

template<ConstructorTag D>
size_t UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<D>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<D>* e) const
{
    return mimir::hash_combine(*e);
}

template size_t
UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<Concept>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<Role>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Role>* e) const;

template<ConstructorTag D>
size_t UniqueDLHasher<const grammar::ChoiceImpl<D>*>::operator()(const grammar::ChoiceImpl<D>* e) const
{
    return mimir::hash_combine(e->get_choice());
}

template size_t UniqueDLHasher<const grammar::ChoiceImpl<Concept>*>::operator()(const grammar::ChoiceImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::ChoiceImpl<Role>*>::operator()(const grammar::ChoiceImpl<Role>* e) const;

/* Concepts */

size_t UniqueDLHasher<const grammar::ConceptBotImpl*>::operator()(const grammar::ConceptBotImpl* e) const { return mimir::hash_combine(e); }

size_t UniqueDLHasher<const grammar::ConceptTopImpl*>::operator()(const grammar::ConceptTopImpl* e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<const grammar::ConceptAtomicStateImpl<P>*>::operator()(const grammar::ConceptAtomicStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::ConceptAtomicStateImpl<Static>*>::operator()(const grammar::ConceptAtomicStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptAtomicStateImpl<Fluent>*>::operator()(const grammar::ConceptAtomicStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptAtomicStateImpl<Derived>*>::operator()(const grammar::ConceptAtomicStateImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniqueDLHasher<const grammar::ConceptAtomicGoalImpl<P>*>::operator()(const grammar::ConceptAtomicGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::ConceptAtomicGoalImpl<Static>*>::operator()(const grammar::ConceptAtomicGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptAtomicGoalImpl<Fluent>*>::operator()(const grammar::ConceptAtomicGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptAtomicGoalImpl<Derived>*>::operator()(const grammar::ConceptAtomicGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const grammar::ConceptIntersectionImpl*>::operator()(const grammar::ConceptIntersectionImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

size_t UniqueDLHasher<const grammar::ConceptUnionImpl*>::operator()(const grammar::ConceptUnionImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

size_t UniqueDLHasher<const grammar::ConceptNegationImpl*>::operator()(const grammar::ConceptNegationImpl* e) const
{
    return mimir::hash_combine(e->get_concept());
}

size_t UniqueDLHasher<const grammar::ConceptValueRestrictionImpl*>::operator()(const grammar::ConceptValueRestrictionImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const grammar::ConceptExistentialQuantificationImpl*>::operator()(const grammar::ConceptExistentialQuantificationImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const grammar::ConceptRoleValueMapContainmentImpl*>::operator()(const grammar::ConceptRoleValueMapContainmentImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const grammar::ConceptRoleValueMapEqualityImpl*>::operator()(const grammar::ConceptRoleValueMapEqualityImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const grammar::ConceptNominalImpl*>::operator()(const grammar::ConceptNominalImpl* e) const
{
    return mimir::hash_combine(e->get_object());
}

/* Roles */

size_t UniqueDLHasher<const grammar::RoleUniversalImpl*>::operator()(const grammar::RoleUniversalImpl* e) const { return mimir::hash_combine(e); }

template<PredicateTag P>
size_t UniqueDLHasher<const grammar::RoleAtomicStateImpl<P>*>::operator()(const grammar::RoleAtomicStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::RoleAtomicStateImpl<Static>*>::operator()(const grammar::RoleAtomicStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::RoleAtomicStateImpl<Fluent>*>::operator()(const grammar::RoleAtomicStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::RoleAtomicStateImpl<Derived>*>::operator()(const grammar::RoleAtomicStateImpl<Derived>* e) const;

template<PredicateTag P>
size_t UniqueDLHasher<const grammar::RoleAtomicGoalImpl<P>*>::operator()(const grammar::RoleAtomicGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->is_negated(), e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::RoleAtomicGoalImpl<Static>*>::operator()(const grammar::RoleAtomicGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::RoleAtomicGoalImpl<Fluent>*>::operator()(const grammar::RoleAtomicGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::RoleAtomicGoalImpl<Derived>*>::operator()(const grammar::RoleAtomicGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const grammar::RoleIntersectionImpl*>::operator()(const grammar::RoleIntersectionImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const grammar::RoleUnionImpl*>::operator()(const grammar::RoleUnionImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const grammar::RoleComplementImpl*>::operator()(const grammar::RoleComplementImpl* e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<const grammar::RoleInverseImpl*>::operator()(const grammar::RoleInverseImpl* e) const { return mimir::hash_combine(e->get_role()); }

size_t UniqueDLHasher<const grammar::RoleCompositionImpl*>::operator()(const grammar::RoleCompositionImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

size_t UniqueDLHasher<const grammar::RoleTransitiveClosureImpl*>::operator()(const grammar::RoleTransitiveClosureImpl* e) const
{
    return mimir::hash_combine(e->get_role());
}

size_t UniqueDLHasher<const grammar::RoleReflexiveTransitiveClosureImpl*>::operator()(const grammar::RoleReflexiveTransitiveClosureImpl* e) const
{
    return mimir::hash_combine(e->get_role());
}

size_t UniqueDLHasher<const grammar::RoleRestrictionImpl*>::operator()(const grammar::RoleRestrictionImpl* e) const
{
    return mimir::hash_combine(e->get_role(), e->get_concept());
}

size_t UniqueDLHasher<const grammar::RoleIdentityImpl*>::operator()(const grammar::RoleIdentityImpl* e) const { return mimir::hash_combine(e->get_concept()); }

}
