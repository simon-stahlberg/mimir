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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/equal_to.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/hash.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::dl::grammar
{

/**
 * Grammar
 */

template<IsConceptOrRole D>
using NonTerminalFactory = loki::UniqueFactory<NonTerminalImpl<D>, UniqueDLHasher<const NonTerminalImpl<D>*>, UniqueDLEqualTo<const NonTerminalImpl<D>*>>;

template<IsConceptOrRole D>
using ChoiceFactory = loki::UniqueFactory<ChoiceImpl<D>, UniqueDLHasher<const ChoiceImpl<D>*>, UniqueDLEqualTo<const ChoiceImpl<D>*>>;

template<IsConceptOrRole D>
using DerivationRuleFactory =
    loki::UniqueFactory<DerivationRuleImpl<D>, UniqueDLHasher<const DerivationRuleImpl<D>*>, UniqueDLEqualTo<const DerivationRuleImpl<D>*>>;

using ConceptBotFactory = loki::UniqueFactory<ConceptBotImpl, UniqueDLHasher<const ConceptBotImpl*>, UniqueDLEqualTo<const ConceptBotImpl*>>;
using ConceptTopFactory = loki::UniqueFactory<ConceptTopImpl, UniqueDLHasher<const ConceptTopImpl*>, UniqueDLEqualTo<const ConceptTopImpl*>>;
template<PredicateCategory P>
using ConceptAtomicStateFactory =
    loki::UniqueFactory<ConceptAtomicStateImpl<P>, UniqueDLHasher<const ConceptAtomicStateImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptAtomicGoalFactory =
    loki::UniqueFactory<ConceptAtomicGoalImpl<P>, UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>>;
using ConceptIntersectionFactory =
    loki::UniqueFactory<ConceptIntersectionImpl, UniqueDLHasher<const ConceptIntersectionImpl*>, UniqueDLEqualTo<const ConceptIntersectionImpl*>>;
using ConceptUnionFactory = loki::UniqueFactory<ConceptUnionImpl, UniqueDLHasher<const ConceptUnionImpl*>, UniqueDLEqualTo<const ConceptUnionImpl*>>;
using ConceptNegationFactory =
    loki::UniqueFactory<ConceptNegationImpl, UniqueDLHasher<const ConceptNegationImpl*>, UniqueDLEqualTo<const ConceptNegationImpl*>>;
using ConceptValueRestrictionFactory =
    loki::UniqueFactory<ConceptValueRestrictionImpl, UniqueDLHasher<const ConceptValueRestrictionImpl*>, UniqueDLEqualTo<const ConceptValueRestrictionImpl*>>;
using ConceptExistentialQuantificationFactory = loki::UniqueFactory<ConceptExistentialQuantificationImpl,
                                                                    UniqueDLHasher<const ConceptExistentialQuantificationImpl*>,
                                                                    UniqueDLEqualTo<const ConceptExistentialQuantificationImpl*>>;
using ConceptRoleValueMapContainmentFactory = loki::UniqueFactory<ConceptRoleValueMapContainmentImpl,
                                                                  UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>,
                                                                  UniqueDLEqualTo<const ConceptRoleValueMapContainmentImpl*>>;
using ConceptRoleValueMapEqualityFactory = loki::UniqueFactory<ConceptRoleValueMapEqualityImpl,
                                                               UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>,
                                                               UniqueDLEqualTo<const ConceptRoleValueMapEqualityImpl*>>;
using ConceptNominalFactory = loki::UniqueFactory<ConceptNominalImpl, UniqueDLHasher<const ConceptNominalImpl*>, UniqueDLEqualTo<const ConceptNominalImpl*>>;

using RoleUniversalFactory = loki::UniqueFactory<RoleUniversalImpl, UniqueDLHasher<const RoleUniversalImpl*>, UniqueDLEqualTo<const RoleUniversalImpl*>>;
template<PredicateCategory P>
using RoleAtomicStateFactory =
    loki::UniqueFactory<RoleAtomicStateImpl<P>, UniqueDLHasher<const RoleAtomicStateImpl<P>*>, UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using RoleAtomicGoalFactory =
    loki::UniqueFactory<RoleAtomicGoalImpl<P>, UniqueDLHasher<const RoleAtomicGoalImpl<P>*>, UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>>;
using RoleIntersectionFactory =
    loki::UniqueFactory<RoleIntersectionImpl, UniqueDLHasher<const RoleIntersectionImpl*>, UniqueDLEqualTo<const RoleIntersectionImpl*>>;
using RoleUnionFactory = loki::UniqueFactory<RoleUnionImpl, UniqueDLHasher<const RoleUnionImpl*>, UniqueDLEqualTo<const RoleUnionImpl*>>;
using RoleComplementFactory = loki::UniqueFactory<RoleComplementImpl, UniqueDLHasher<const RoleComplementImpl*>, UniqueDLEqualTo<const RoleComplementImpl*>>;
using RoleInverseFactory = loki::UniqueFactory<RoleInverseImpl, UniqueDLHasher<const RoleInverseImpl*>, UniqueDLEqualTo<const RoleInverseImpl*>>;
using RoleCompositionFactory =
    loki::UniqueFactory<RoleCompositionImpl, UniqueDLHasher<const RoleCompositionImpl*>, UniqueDLEqualTo<const RoleCompositionImpl*>>;
using RoleTransitiveClosureFactory =
    loki::UniqueFactory<RoleTransitiveClosureImpl, UniqueDLHasher<const RoleTransitiveClosureImpl*>, UniqueDLEqualTo<const RoleTransitiveClosureImpl*>>;
using RoleReflexiveTransitiveClosureFactory = loki::UniqueFactory<RoleReflexiveTransitiveClosureImpl,
                                                                  UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>,
                                                                  UniqueDLEqualTo<const RoleReflexiveTransitiveClosureImpl*>>;
using RoleRestrictionFactory =
    loki::UniqueFactory<RoleRestrictionImpl, UniqueDLHasher<const RoleRestrictionImpl*>, UniqueDLEqualTo<const RoleRestrictionImpl*>>;
using RoleIdentityFactory = loki::UniqueFactory<RoleIdentityImpl, UniqueDLHasher<const RoleIdentityImpl*>, UniqueDLEqualTo<const RoleIdentityImpl*>>;

using ConstructorRepositories =
    boost::hana::map<boost::hana::pair<boost::hana::type<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ChoiceImpl<Concept>>, ChoiceFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Concept>>, DerivationRuleFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotFactory>,
                     boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopFactory>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateFactory<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateFactory<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateFactory<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Static>>, ConceptAtomicGoalFactory<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Fluent>>, ConceptAtomicGoalFactory<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Derived>>, ConceptAtomicGoalFactory<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionFactory>,
                     boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionFactory>,
                     boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationFactory>,
                     boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionFactory>,
                     boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationFactory>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentFactory>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityFactory>,
                     boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalFactory>,
                     boost::hana::pair<boost::hana::type<NonTerminalImpl<Role>>, NonTerminalFactory<Role>>,
                     boost::hana::pair<boost::hana::type<ChoiceImpl<Role>>, ChoiceFactory<Role>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Role>>, DerivationRuleFactory<Role>>,
                     boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalFactory>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Static>>, RoleAtomicStateFactory<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateFactory<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Derived>>, RoleAtomicStateFactory<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Static>>, RoleAtomicGoalFactory<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Fluent>>, RoleAtomicGoalFactory<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Derived>>, RoleAtomicGoalFactory<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionFactory>,
                     boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionFactory>,
                     boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementFactory>,
                     boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseFactory>,
                     boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionFactory>,
                     boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureFactory>,
                     boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureFactory>,
                     boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionFactory>,
                     boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>>;

extern ConstructorRepositories create_default_constructor_repositories();

}

#endif
