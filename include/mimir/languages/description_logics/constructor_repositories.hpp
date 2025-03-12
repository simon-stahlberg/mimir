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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_REPOSITORIES_HPP_

#include "mimir/languages/description_logics/constructors.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::dl
{

template<typename T>
using SegmentedDLRepository = loki::SegmentedRepository<T>;

using ConceptBotRepository = SegmentedDLRepository<ConceptBotImpl>;
using ConceptTopRepository = SegmentedDLRepository<ConceptTopImpl>;
template<StaticOrFluentOrDerived P>
using ConceptAtomicStateRepositoryImpl = SegmentedDLRepository<ConceptAtomicStateImpl<P>>;
template<StaticOrFluentOrDerived P>
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
template<StaticOrFluentOrDerived P>
using RoleAtomicStateRepositoryImpl = SegmentedDLRepository<RoleAtomicStateImpl<P>>;
template<StaticOrFluentOrDerived P>
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

template<StaticOrFluentOrDerived P>
using BooleanAtomicStateRepository = SegmentedDLRepository<BooleanAtomicStateImpl<P>>;
template<DescriptionLogicCategory D>
using BooleanNonemptyRepository = SegmentedDLRepository<BooleanNonemptyImpl<D>>;

template<DescriptionLogicCategory D>
using NumericalCountRepository = SegmentedDLRepository<NumericalCountImpl<D>>;
using NumericalDistanceRepository = SegmentedDLRepository<NumericalDistanceImpl>;

using HanaConstructorRepositories =
    boost::hana::map<boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,  //
                     boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateRepositoryImpl<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateRepositoryImpl<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateRepositoryImpl<Derived>>,
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
                     boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Static>>, RoleAtomicStateRepositoryImpl<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateRepositoryImpl<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Derived>>, RoleAtomicStateRepositoryImpl<Derived>>,
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
                     boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<Static>>, BooleanAtomicStateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<Fluent>>, BooleanAtomicStateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<Derived>>, BooleanAtomicStateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<Concept>>, BooleanNonemptyRepository<Concept>>,
                     boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<Role>>, BooleanNonemptyRepository<Role>>,
                     boost::hana::pair<boost::hana::type<NumericalCountImpl<Concept>>, NumericalCountRepository<Concept>>,
                     boost::hana::pair<boost::hana::type<NumericalCountImpl<Role>>, NumericalCountRepository<Role>>,
                     boost::hana::pair<boost::hana::type<NumericalDistanceImpl>, NumericalDistanceRepository>>;

class ConstructorRepositories
{
private:
    HanaConstructorRepositories m_repositories;

public:
    ConstructorRepositories() = default;
    ConstructorRepositories(const ConstructorRepositories& other) = delete;
    ConstructorRepositories& operator=(const ConstructorRepositories& other) = delete;
    ConstructorRepositories(ConstructorRepositories&& other) = default;
    ConstructorRepositories& operator=(ConstructorRepositories&& other) = default;

    HanaConstructorRepositories& get_repositories();
    const HanaConstructorRepositories& get_repositories() const;

    /* Concepts */
    Constructor<Concept> get_or_create_concept_bot();
    Constructor<Concept> get_or_create_concept_top();
    template<StaticOrFluentOrDerived P>
    Constructor<Concept> get_or_create_concept_atomic_state(Predicate<P> predicate);
    template<StaticOrFluentOrDerived P>
    Constructor<Concept> get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated);
    Constructor<Concept> get_or_create_concept_intersection(Constructor<Concept> left_concept, Constructor<Concept> right_concept);
    Constructor<Concept> get_or_create_concept_union(Constructor<Concept> left_concept, Constructor<Concept> right_concept);
    Constructor<Concept> get_or_create_concept_negation(Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_value_restriction(Constructor<Role> role, Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_existential_quantification(Constructor<Role> role, Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_role_value_map_containment(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Concept> get_or_create_concept_role_value_map_equality(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Concept> get_or_create_concept_nominal(Object object);

    /* Roles */
    Constructor<Role> get_or_create_role_universal();
    template<StaticOrFluentOrDerived P>
    Constructor<Role> get_or_create_role_atomic_state(Predicate<P> predicate);
    template<StaticOrFluentOrDerived P>
    Constructor<Role> get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated);
    Constructor<Role> get_or_create_role_intersection(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Role> get_or_create_role_union(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Role> get_or_create_role_complement(Constructor<Role> role);
    Constructor<Role> get_or_create_role_inverse(Constructor<Role> role);
    Constructor<Role> get_or_create_role_composition(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Role> get_or_create_role_transitive_closure(Constructor<Role> role);
    Constructor<Role> get_or_create_role_reflexive_transitive_closure(Constructor<Role> role);
    Constructor<Role> get_or_create_role_restriction(Constructor<Role> role, Constructor<Concept> concept_);
    Constructor<Role> get_or_create_role_identity(Constructor<Concept> concept_);

    /* Booleans */
    template<StaticOrFluentOrDerived P>
    Constructor<Boolean> get_or_create_boolean_atomic_state(Predicate<P> predicate);
    template<DescriptionLogicCategory D>
    Constructor<Boolean> get_or_create_boolean_nonempty(Constructor<D> constructor);

    /* Numericals */
    template<DescriptionLogicCategory D>
    Constructor<Numerical> get_or_create_numerical_count(Constructor<D> constructor);
    Constructor<Numerical> get_or_create_numerical_distance(Constructor<Concept> left_concept, Constructor<Role> role, Constructor<Concept> right_concept);
};

}

#endif
