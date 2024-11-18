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

using ConceptBotRepository = loki::UniqueFactory<ConceptBotImpl, UniqueDLHasher<const ConceptBotImpl*>, UniqueDLEqualTo<const ConceptBotImpl*>>;
using ConceptTopRepository = loki::UniqueFactory<ConceptTopImpl, UniqueDLHasher<const ConceptTopImpl*>, UniqueDLEqualTo<const ConceptTopImpl*>>;
template<PredicateCategory P>
using ConceptAtomicStateRepository =
    loki::UniqueFactory<ConceptAtomicStateImpl<P>, UniqueDLHasher<const ConceptAtomicStateImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptAtomicGoalRepository =
    loki::UniqueFactory<ConceptAtomicGoalImpl<P>, UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>>;
using ConceptIntersectionRepository =
    loki::UniqueFactory<ConceptIntersectionImpl, UniqueDLHasher<const ConceptIntersectionImpl*>, UniqueDLEqualTo<const ConceptIntersectionImpl*>>;
using ConceptUnionRepository = loki::UniqueFactory<ConceptUnionImpl, UniqueDLHasher<const ConceptUnionImpl*>, UniqueDLEqualTo<const ConceptUnionImpl*>>;
using ConceptNegationRepository =
    loki::UniqueFactory<ConceptNegationImpl, UniqueDLHasher<const ConceptNegationImpl*>, UniqueDLEqualTo<const ConceptNegationImpl*>>;
using ConceptValueRestrictionRepository =
    loki::UniqueFactory<ConceptValueRestrictionImpl, UniqueDLHasher<const ConceptValueRestrictionImpl*>, UniqueDLEqualTo<const ConceptValueRestrictionImpl*>>;
using ConceptExistentialQuantificationRepository = loki::UniqueFactory<ConceptExistentialQuantificationImpl,
                                                                    UniqueDLHasher<const ConceptExistentialQuantificationImpl*>,
                                                                    UniqueDLEqualTo<const ConceptExistentialQuantificationImpl*>>;
using ConceptRoleValueMapContainmentRepository = loki::UniqueFactory<ConceptRoleValueMapContainmentImpl,
                                                                  UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>,
                                                                  UniqueDLEqualTo<const ConceptRoleValueMapContainmentImpl*>>;
using ConceptRoleValueMapEqualityRepository = loki::UniqueFactory<ConceptRoleValueMapEqualityImpl,
                                                               UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>,
                                                               UniqueDLEqualTo<const ConceptRoleValueMapEqualityImpl*>>;
using ConceptNominalRepository = loki::UniqueFactory<ConceptNominalImpl, UniqueDLHasher<const ConceptNominalImpl*>, UniqueDLEqualTo<const ConceptNominalImpl*>>;

using RoleUniversalRepository = loki::UniqueFactory<RoleUniversalImpl, UniqueDLHasher<const RoleUniversalImpl*>, UniqueDLEqualTo<const RoleUniversalImpl*>>;
template<PredicateCategory P>
using RoleAtomicStateRepository =
    loki::UniqueFactory<RoleAtomicStateImpl<P>, UniqueDLHasher<const RoleAtomicStateImpl<P>*>, UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using RoleAtomicGoalRepository =
    loki::UniqueFactory<RoleAtomicGoalImpl<P>, UniqueDLHasher<const RoleAtomicGoalImpl<P>*>, UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>>;
using RoleIntersectionRepository =
    loki::UniqueFactory<RoleIntersectionImpl, UniqueDLHasher<const RoleIntersectionImpl*>, UniqueDLEqualTo<const RoleIntersectionImpl*>>;
using RoleUnionRepository = loki::UniqueFactory<RoleUnionImpl, UniqueDLHasher<const RoleUnionImpl*>, UniqueDLEqualTo<const RoleUnionImpl*>>;
using RoleComplementRepository = loki::UniqueFactory<RoleComplementImpl, UniqueDLHasher<const RoleComplementImpl*>, UniqueDLEqualTo<const RoleComplementImpl*>>;
using RoleInverseRepository = loki::UniqueFactory<RoleInverseImpl, UniqueDLHasher<const RoleInverseImpl*>, UniqueDLEqualTo<const RoleInverseImpl*>>;
using RoleCompositionRepository =
    loki::UniqueFactory<RoleCompositionImpl, UniqueDLHasher<const RoleCompositionImpl*>, UniqueDLEqualTo<const RoleCompositionImpl*>>;
using RoleTransitiveClosureRepository =
    loki::UniqueFactory<RoleTransitiveClosureImpl, UniqueDLHasher<const RoleTransitiveClosureImpl*>, UniqueDLEqualTo<const RoleTransitiveClosureImpl*>>;
using RoleReflexiveTransitiveClosureRepository = loki::UniqueFactory<RoleReflexiveTransitiveClosureImpl,
                                                                  UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>,
                                                                  UniqueDLEqualTo<const RoleReflexiveTransitiveClosureImpl*>>;
using RoleRestrictionRepository =
    loki::UniqueFactory<RoleRestrictionImpl, UniqueDLHasher<const RoleRestrictionImpl*>, UniqueDLEqualTo<const RoleRestrictionImpl*>>;
using RoleIdentityFactory = loki::UniqueFactory<RoleIdentityImpl, UniqueDLHasher<const RoleIdentityImpl*>, UniqueDLEqualTo<const RoleIdentityImpl*>>;

using ConstructorTypeToRepository =
    boost::hana::map<boost::hana::pair<boost::hana::type<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ChoiceImpl<Concept>>, ChoiceFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Concept>>, DerivationRuleFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,
                     boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Static>>, ConceptAtomicGoalRepository<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Fluent>>, ConceptAtomicGoalRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Derived>>, ConceptAtomicGoalRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalRepository>,
                     boost::hana::pair<boost::hana::type<NonTerminalImpl<Role>>, NonTerminalFactory<Role>>,
                     boost::hana::pair<boost::hana::type<ChoiceImpl<Role>>, ChoiceFactory<Role>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Role>>, DerivationRuleFactory<Role>>,
                     boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Static>>, RoleAtomicStateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Derived>>, RoleAtomicStateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Static>>, RoleAtomicGoalRepository<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Fluent>>, RoleAtomicGoalRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Derived>>, RoleAtomicGoalRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionRepository>,
                     boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementRepository>,
                     boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseRepository>,
                     boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionRepository>,
                     boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>>;

extern ConstructorTypeToRepository create_default_constructor_type_to_repository();

}

#endif
