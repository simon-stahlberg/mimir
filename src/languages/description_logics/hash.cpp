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

template<PredicateCategory P>
size_t UniqueDLHasher<const ConceptPredicateStateImpl<P>*>::operator()(const ConceptPredicateStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const ConceptPredicateStateImpl<Static>*>::operator()(const ConceptPredicateStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const ConceptPredicateStateImpl<Fluent>*>::operator()(const ConceptPredicateStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const ConceptPredicateStateImpl<Derived>*>::operator()(const ConceptPredicateStateImpl<Derived>* e) const;

template<PredicateCategory P>
size_t UniqueDLHasher<const ConceptPredicateGoalImpl<P>*>::operator()(const ConceptPredicateGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const ConceptPredicateGoalImpl<Static>*>::operator()(const ConceptPredicateGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const ConceptPredicateGoalImpl<Fluent>*>::operator()(const ConceptPredicateGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const ConceptPredicateGoalImpl<Derived>*>::operator()(const ConceptPredicateGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const ConceptAndImpl*>::operator()(const ConceptAndImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

/* Roles */

template<PredicateCategory P>
size_t UniqueDLHasher<const RolePredicateStateImpl<P>*>::operator()(const RolePredicateStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const RolePredicateStateImpl<Static>*>::operator()(const RolePredicateStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const RolePredicateStateImpl<Fluent>*>::operator()(const RolePredicateStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const RolePredicateStateImpl<Derived>*>::operator()(const RolePredicateStateImpl<Derived>* e) const;

template<PredicateCategory P>
size_t UniqueDLHasher<const RolePredicateGoalImpl<P>*>::operator()(const RolePredicateGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const RolePredicateGoalImpl<Static>*>::operator()(const RolePredicateGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const RolePredicateGoalImpl<Fluent>*>::operator()(const RolePredicateGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const RolePredicateGoalImpl<Derived>*>::operator()(const RolePredicateGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const RoleAndImpl*>::operator()(const RoleAndImpl* e) const { return mimir::hash_combine(e->get_role_left(), e->get_role_right()); }

/**
 * DL grammar constructors
 */

template<IsConceptOrRole D>
size_t UniqueDLHasher<const grammar::DerivationRuleImpl<D>*>::operator()(const grammar::DerivationRuleImpl<D>* e) const
{
    return mimir::hash_combine(e->get_choices());
}

template size_t UniqueDLHasher<const grammar::DerivationRuleImpl<Concept>*>::operator()(const grammar::DerivationRuleImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::DerivationRuleImpl<Role>*>::operator()(const grammar::DerivationRuleImpl<Role>* e) const;

template<IsConceptOrRole D>
size_t UniqueDLHasher<const grammar::NonTerminalImpl<D>*>::operator()(const grammar::NonTerminalImpl<D>* e) const
{
    return mimir::hash_combine(e->get_name());
}

template size_t UniqueDLHasher<const grammar::NonTerminalImpl<Concept>*>::operator()(const grammar::NonTerminalImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::NonTerminalImpl<Role>*>::operator()(const grammar::NonTerminalImpl<Role>* e) const;

template<IsConceptOrRole D>
size_t UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<D>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<D>* e) const
{
    return mimir::hash_combine(*e);
}

template size_t
UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<Concept>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::ConstructorOrNonTerminalChoice<Role>*>::operator()(const grammar::ConstructorOrNonTerminalChoice<Role>* e) const;

template<IsConceptOrRole D>
size_t UniqueDLHasher<const grammar::ChoiceImpl<D>*>::operator()(const grammar::ChoiceImpl<D>* e) const
{
    return mimir::hash_combine(e->get_choice());
}

template size_t UniqueDLHasher<const grammar::ChoiceImpl<Concept>*>::operator()(const grammar::ChoiceImpl<Concept>* e) const;
template size_t UniqueDLHasher<const grammar::ChoiceImpl<Role>*>::operator()(const grammar::ChoiceImpl<Role>* e) const;

/* Concepts */

template<PredicateCategory P>
size_t UniqueDLHasher<const grammar::ConceptPredicateStateImpl<P>*>::operator()(const grammar::ConceptPredicateStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::ConceptPredicateStateImpl<Static>*>::operator()(const grammar::ConceptPredicateStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptPredicateStateImpl<Fluent>*>::operator()(const grammar::ConceptPredicateStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptPredicateStateImpl<Derived>*>::operator()(const grammar::ConceptPredicateStateImpl<Derived>* e) const;

template<PredicateCategory P>
size_t UniqueDLHasher<const grammar::ConceptPredicateGoalImpl<P>*>::operator()(const grammar::ConceptPredicateGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::ConceptPredicateGoalImpl<Static>*>::operator()(const grammar::ConceptPredicateGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptPredicateGoalImpl<Fluent>*>::operator()(const grammar::ConceptPredicateGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::ConceptPredicateGoalImpl<Derived>*>::operator()(const grammar::ConceptPredicateGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const grammar::ConceptAndImpl*>::operator()(const grammar::ConceptAndImpl* e) const
{
    return mimir::hash_combine(e->get_concept_left(), e->get_concept_right());
}

/* Roles */

template<PredicateCategory P>
size_t UniqueDLHasher<const grammar::RolePredicateStateImpl<P>*>::operator()(const grammar::RolePredicateStateImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::RolePredicateStateImpl<Static>*>::operator()(const grammar::RolePredicateStateImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::RolePredicateStateImpl<Fluent>*>::operator()(const grammar::RolePredicateStateImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::RolePredicateStateImpl<Derived>*>::operator()(const grammar::RolePredicateStateImpl<Derived>* e) const;

template<PredicateCategory P>
size_t UniqueDLHasher<const grammar::RolePredicateGoalImpl<P>*>::operator()(const grammar::RolePredicateGoalImpl<P>* e) const
{
    return mimir::hash_combine(e->get_predicate());
}

template size_t UniqueDLHasher<const grammar::RolePredicateGoalImpl<Static>*>::operator()(const grammar::RolePredicateGoalImpl<Static>* e) const;
template size_t UniqueDLHasher<const grammar::RolePredicateGoalImpl<Fluent>*>::operator()(const grammar::RolePredicateGoalImpl<Fluent>* e) const;
template size_t UniqueDLHasher<const grammar::RolePredicateGoalImpl<Derived>*>::operator()(const grammar::RolePredicateGoalImpl<Derived>* e) const;

size_t UniqueDLHasher<const grammar::RoleAndImpl*>::operator()(const grammar::RoleAndImpl* e) const
{
    return mimir::hash_combine(e->get_role_left(), e->get_role_right());
}

}
