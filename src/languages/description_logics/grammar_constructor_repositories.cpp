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

#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"

namespace mimir::dl::grammar
{

ConstructorTagToRepository create_default_constructor_type_to_repository()
{
    return boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type_c<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept> {}),
        boost::hana::make_pair(boost::hana::type_c<ConstructorOrNonTerminalImpl<Concept>>, ChoiceFactory<Concept> {}),
        boost::hana::make_pair(boost::hana::type_c<DerivationRuleImpl<Concept>>, DerivationRuleFactory<Concept> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptBotImpl>, ConceptBotRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptTopImpl>, ConceptTopRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Static>>, ConceptAtomicGoalRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Fluent>>, ConceptAtomicGoalRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Derived>>, ConceptAtomicGoalRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptIntersectionImpl>, ConceptIntersectionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptUnionImpl>, ConceptUnionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptNegationImpl>, ConceptNegationRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository {}),
        boost::hana::make_pair(boost::hana::type_c<ConceptNominalImpl>, ConceptNominalRepository {}),
        boost::hana::make_pair(boost::hana::type_c<NonTerminalImpl<Role>>, NonTerminalFactory<Role> {}),
        boost::hana::make_pair(boost::hana::type_c<ConstructorOrNonTerminalImpl<Role>>, ChoiceFactory<Role> {}),
        boost::hana::make_pair(boost::hana::type_c<DerivationRuleImpl<Role>>, DerivationRuleFactory<Role> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleUniversalImpl>, RoleUniversalRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Static>>, RoleAtomicStateRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Derived>>, RoleAtomicStateRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Static>>, RoleAtomicGoalRepository<Static> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Fluent>>, RoleAtomicGoalRepository<Fluent> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Derived>>, RoleAtomicGoalRepository<Derived> {}),
        boost::hana::make_pair(boost::hana::type_c<RoleIntersectionImpl>, RoleIntersectionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleUnionImpl>, RoleUnionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleComplementImpl>, RoleComplementRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleInverseImpl>, RoleInverseRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleCompositionImpl>, RoleCompositionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleRestrictionImpl>, RoleRestrictionRepository {}),
        boost::hana::make_pair(boost::hana::type_c<RoleIdentityImpl>, RoleIdentityFactory {}));
}

}
