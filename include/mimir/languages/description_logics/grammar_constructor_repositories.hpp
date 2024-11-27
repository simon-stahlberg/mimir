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

template<typename T>
using SegmentedDLRepository = loki::SegmentedRepository<T, UniqueDLHasher<const T*>, UniqueDLEqualTo<const T*>>;

template<ConstructorTag D>
using NonTerminalFactory = SegmentedDLRepository<NonTerminalImpl<D>>;

template<ConstructorTag D>
using ChoiceFactory = SegmentedDLRepository<ConstructorOrNonTerminalImpl<D>>;

template<ConstructorTag D>
using DerivationRuleFactory = SegmentedDLRepository<DerivationRuleImpl<D>>;

using ConceptBotRepository = SegmentedDLRepository<ConceptBotImpl>;
using ConceptTopRepository = SegmentedDLRepository<ConceptTopImpl>;
template<PredicateTag P>
using ConceptAtomicStateRepository = SegmentedDLRepository<ConceptAtomicStateImpl<P>>;
template<PredicateTag P>
using ConceptAtomicGoalRepository = SegmentedDLRepository<ConceptAtomicGoalImpl<P>>;
using ConceptIntersectionRepository = SegmentedDLRepository<ConceptIntersectionImpl>;
using ConceptUnionRepository = SegmentedDLRepository<ConceptUnionImpl>;
using ConceptNegationRepository = SegmentedDLRepository<ConceptNegationImpl>;
using ConceptValueRestrictionRepository = SegmentedDLRepository<ConceptValueRestrictionImpl>;
using ConceptExistentialQuantificationRepository = SegmentedDLRepository<ConceptExistentialQuantificationImpl>;
using ConceptRoleValueMapContainmentRepository = SegmentedDLRepository<ConceptRoleValueMapContainmentImpl>;
using ConceptRoleValueMapEqualityRepository = SegmentedDLRepository<ConceptRoleValueMapEqualityImpl>;
using ConceptNominalRepository = SegmentedDLRepository<ConceptNominalImpl>;

using RoleUniversalRepository = SegmentedDLRepository<RoleUniversalImpl>;
template<PredicateTag P>
using RoleAtomicStateRepository = SegmentedDLRepository<RoleAtomicStateImpl<P>>;
template<PredicateTag P>
using RoleAtomicGoalRepository = SegmentedDLRepository<RoleAtomicGoalImpl<P>>;
using RoleIntersectionRepository = SegmentedDLRepository<RoleIntersectionImpl>;
using RoleUnionRepository = SegmentedDLRepository<RoleUnionImpl>;
using RoleComplementRepository = SegmentedDLRepository<RoleComplementImpl>;
using RoleInverseRepository = SegmentedDLRepository<RoleInverseImpl>;
using RoleCompositionRepository = SegmentedDLRepository<RoleCompositionImpl>;
using RoleTransitiveClosureRepository = SegmentedDLRepository<RoleTransitiveClosureImpl>;
using RoleReflexiveTransitiveClosureRepository = SegmentedDLRepository<RoleReflexiveTransitiveClosureImpl>;
using RoleRestrictionRepository = SegmentedDLRepository<RoleRestrictionImpl>;
using RoleIdentityFactory = SegmentedDLRepository<RoleIdentityImpl>;

using ConstructorTagToRepository =
    boost::hana::map<boost::hana::pair<boost::hana::type<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<Concept>>, ChoiceFactory<Concept>>,
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
                     boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<Role>>, ChoiceFactory<Role>>,
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

extern ConstructorTagToRepository create_default_constructor_type_to_repository();

}

#endif
