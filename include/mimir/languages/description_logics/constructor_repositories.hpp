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

namespace mimir::languages::dl
{

using ConceptBotRepository = loki::SegmentedRepository<ConceptBotImpl>;
using ConceptTopRepository = loki::SegmentedRepository<ConceptTopImpl>;
template<formalism::StaticOrFluentOrDerived P>
using ConceptAtomicStateRepositoryImpl = loki::SegmentedRepository<ConceptAtomicStateImpl<P>>;
template<formalism::StaticOrFluentOrDerived P>
using ConceptAtomicGoalRepository = loki::SegmentedRepository<ConceptAtomicGoalImpl<P>>;
using ConceptIntersectionRepository = loki::SegmentedRepository<ConceptIntersectionImpl>;
using ConceptUnionRepository = loki::SegmentedRepository<ConceptUnionImpl>;
using ConceptNegationRepository = loki::SegmentedRepository<ConceptNegationImpl>;
using ConceptValueRestrictionRepository = loki::SegmentedRepository<ConceptValueRestrictionImpl>;
using ConceptExistentialQuantificationRepository = loki::SegmentedRepository<ConceptExistentialQuantificationImpl>;
using ConceptRoleValueMapContainmentRepository = loki::SegmentedRepository<ConceptRoleValueMapContainmentImpl>;
using ConceptRoleValueMapEqualityRepository = loki::SegmentedRepository<ConceptRoleValueMapEqualityImpl>;
using ConceptNominalRepository = loki::SegmentedRepository<ConceptNominalImpl>;

using RoleUniversalRepository = loki::SegmentedRepository<RoleUniversalImpl>;
template<formalism::StaticOrFluentOrDerived P>
using RoleAtomicStateRepositoryImpl = loki::SegmentedRepository<RoleAtomicStateImpl<P>>;
template<formalism::StaticOrFluentOrDerived P>
using RoleAtomicGoalRepository = loki::SegmentedRepository<RoleAtomicGoalImpl<P>>;
using RoleIntersectionRepository = loki::SegmentedRepository<RoleIntersectionImpl>;
using RoleUnionRepository = loki::SegmentedRepository<RoleUnionImpl>;
using RoleComplementRepository = loki::SegmentedRepository<RoleComplementImpl>;
using RoleInverseRepository = loki::SegmentedRepository<RoleInverseImpl>;
using RoleCompositionRepository = loki::SegmentedRepository<RoleCompositionImpl>;
using RoleTransitiveClosureRepository = loki::SegmentedRepository<RoleTransitiveClosureImpl>;
using RoleReflexiveTransitiveClosureRepository = loki::SegmentedRepository<RoleReflexiveTransitiveClosureImpl>;
using RoleRestrictionRepository = loki::SegmentedRepository<RoleRestrictionImpl>;
using RoleIdentityFactory = loki::SegmentedRepository<RoleIdentityImpl>;

template<formalism::StaticOrFluentOrDerived P>
using BooleanAtomicStateRepository = loki::SegmentedRepository<BooleanAtomicStateImpl<P>>;
template<DescriptionLogicCategory D>
using BooleanNonemptyRepository = loki::SegmentedRepository<BooleanNonemptyImpl<D>>;

template<DescriptionLogicCategory D>
using NumericalCountRepository = loki::SegmentedRepository<NumericalCountImpl<D>>;
using NumericalDistanceRepository = loki::SegmentedRepository<NumericalDistanceImpl>;

using HanaRepositories =
    boost::hana::map<boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,  //
                     boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::Static>>, ConceptAtomicStateRepositoryImpl<formalism::Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::Fluent>>, ConceptAtomicStateRepositoryImpl<formalism::Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::Derived>>, ConceptAtomicStateRepositoryImpl<formalism::Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::Static>>, ConceptAtomicGoalRepository<formalism::Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::Fluent>>, ConceptAtomicGoalRepository<formalism::Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::Derived>>, ConceptAtomicGoalRepository<formalism::Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalRepository>,
                     boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::Static>>, RoleAtomicStateRepositoryImpl<formalism::Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::Fluent>>, RoleAtomicStateRepositoryImpl<formalism::Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::Derived>>, RoleAtomicStateRepositoryImpl<formalism::Derived>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::Static>>, RoleAtomicGoalRepository<formalism::Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::Fluent>>, RoleAtomicGoalRepository<formalism::Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::Derived>>, RoleAtomicGoalRepository<formalism::Derived>>,
                     boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionRepository>,
                     boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementRepository>,
                     boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseRepository>,
                     boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionRepository>,
                     boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::Static>>, BooleanAtomicStateRepository<formalism::Static>>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::Fluent>>, BooleanAtomicStateRepository<formalism::Fluent>>,
                     boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::Derived>>, BooleanAtomicStateRepository<formalism::Derived>>,
                     boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<Concept>>, BooleanNonemptyRepository<Concept>>,
                     boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<Role>>, BooleanNonemptyRepository<Role>>,
                     boost::hana::pair<boost::hana::type<NumericalCountImpl<Concept>>, NumericalCountRepository<Concept>>,
                     boost::hana::pair<boost::hana::type<NumericalCountImpl<Role>>, NumericalCountRepository<Role>>,
                     boost::hana::pair<boost::hana::type<NumericalDistanceImpl>, NumericalDistanceRepository>>;

class Repositories
{
private:
    HanaRepositories m_repositories;

public:
    Repositories() = default;
    Repositories(const Repositories& other) = delete;
    Repositories& operator=(const Repositories& other) = delete;
    Repositories(Repositories&& other) = default;
    Repositories& operator=(Repositories&& other) = default;

    HanaRepositories& get_repositories();
    const HanaRepositories& get_repositories() const;

    template<FeatureCategory D>
    Constructor<D> get_or_create(const std::string& sentence, const formalism::DomainImpl& domain);

    /* Concepts */
    Constructor<Concept> get_or_create_concept_bot();
    Constructor<Concept> get_or_create_concept_top();
    template<formalism::StaticOrFluentOrDerived P>
    Constructor<Concept> get_or_create_concept_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::StaticOrFluentOrDerived P>
    Constructor<Concept> get_or_create_concept_atomic_goal(formalism::Predicate<P> predicate, bool is_negated);
    Constructor<Concept> get_or_create_concept_intersection(Constructor<Concept> left_concept, Constructor<Concept> right_concept);
    Constructor<Concept> get_or_create_concept_union(Constructor<Concept> left_concept, Constructor<Concept> right_concept);
    Constructor<Concept> get_or_create_concept_negation(Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_value_restriction(Constructor<Role> role, Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_existential_quantification(Constructor<Role> role, Constructor<Concept> concept_);
    Constructor<Concept> get_or_create_concept_role_value_map_containment(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Concept> get_or_create_concept_role_value_map_equality(Constructor<Role> left_role, Constructor<Role> right_role);
    Constructor<Concept> get_or_create_concept_nominal(formalism::Object object);

    /* Roles */
    Constructor<Role> get_or_create_role_universal();
    template<formalism::StaticOrFluentOrDerived P>
    Constructor<Role> get_or_create_role_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::StaticOrFluentOrDerived P>
    Constructor<Role> get_or_create_role_atomic_goal(formalism::Predicate<P> predicate, bool is_negated);
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
    template<formalism::StaticOrFluentOrDerived P>
    Constructor<Boolean> get_or_create_boolean_atomic_state(formalism::Predicate<P> predicate);
    template<DescriptionLogicCategory D>
    Constructor<Boolean> get_or_create_boolean_nonempty(Constructor<D> constructor);

    /* Numericals */
    template<DescriptionLogicCategory D>
    Constructor<Numerical> get_or_create_numerical_count(Constructor<D> constructor);
    Constructor<Numerical> get_or_create_numerical_distance(Constructor<Concept> left_concept, Constructor<Role> role, Constructor<Concept> right_concept);
};

}

#endif
