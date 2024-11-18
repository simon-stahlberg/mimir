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

ConstructorRepositories create_default_constructor_repositories()
{
    return boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ChoiceImpl<Concept>>, ChoiceFactory<Concept> {}),
                                 boost::hana::make_pair(boost::hana::type_c<DerivationRuleImpl<Concept>>, DerivationRuleFactory<Concept> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptBotImpl>, ConceptBotFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptTopImpl>, ConceptTopFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Static>>, ConceptAtomicGoalFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Fluent>>, ConceptAtomicGoalFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptAtomicGoalImpl<Derived>>, ConceptAtomicGoalFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptIntersectionImpl>, ConceptIntersectionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptUnionImpl>, ConceptUnionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptNegationImpl>, ConceptNegationFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptValueRestrictionImpl>, ConceptValueRestrictionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<ConceptNominalImpl>, ConceptNominalFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<NonTerminalImpl<Role>>, NonTerminalFactory<Role> {}),
                                 boost::hana::make_pair(boost::hana::type_c<ChoiceImpl<Role>>, ChoiceFactory<Role> {}),
                                 boost::hana::make_pair(boost::hana::type_c<DerivationRuleImpl<Role>>, DerivationRuleFactory<Role> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleUniversalImpl>, RoleUniversalFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Static>>, RoleAtomicStateFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicStateImpl<Derived>>, RoleAtomicStateFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Static>>, RoleAtomicGoalFactory<Static> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Fluent>>, RoleAtomicGoalFactory<Fluent> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleAtomicGoalImpl<Derived>>, RoleAtomicGoalFactory<Derived> {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleIntersectionImpl>, RoleIntersectionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleUnionImpl>, RoleUnionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleComplementImpl>, RoleComplementFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleInverseImpl>, RoleInverseFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleCompositionImpl>, RoleCompositionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleTransitiveClosureImpl>, RoleTransitiveClosureFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleRestrictionImpl>, RoleRestrictionFactory {}),
                                 boost::hana::make_pair(boost::hana::type_c<RoleIdentityImpl>, RoleIdentityFactory {}));
}

}
