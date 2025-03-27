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
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicStateRepositoryImpl = loki::SegmentedRepository<ConceptAtomicStateImpl<P>>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicStateRepositoryImpl = loki::SegmentedRepository<RoleAtomicStateImpl<P>>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
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

template<formalism::IsStaticOrFluentOrDerivedTag P>
using BooleanAtomicStateRepository = loki::SegmentedRepository<BooleanAtomicStateImpl<P>>;
template<IsConceptOrRoleTag D>
using BooleanNonemptyRepository = loki::SegmentedRepository<BooleanNonemptyImpl<D>>;

template<IsConceptOrRoleTag D>
using NumericalCountRepository = loki::SegmentedRepository<NumericalCountImpl<D>>;
using NumericalDistanceRepository = loki::SegmentedRepository<NumericalDistanceImpl>;

using HanaRepositories = boost::hana::map<
    boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,  //
    boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::StaticTag>>, ConceptAtomicStateRepositoryImpl<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::FluentTag>>, ConceptAtomicStateRepositoryImpl<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::DerivedTag>>, ConceptAtomicStateRepositoryImpl<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::StaticTag>>, ConceptAtomicGoalRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::FluentTag>>, ConceptAtomicGoalRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::DerivedTag>>, ConceptAtomicGoalRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionRepository>,
    boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionRepository>,
    boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationRepository>,
    boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository>,
    boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository>,
    boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository>,
    boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository>,
    boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalRepository>,
    boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::StaticTag>>, RoleAtomicStateRepositoryImpl<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::FluentTag>>, RoleAtomicStateRepositoryImpl<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::DerivedTag>>, RoleAtomicStateRepositoryImpl<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::StaticTag>>, RoleAtomicGoalRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::FluentTag>>, RoleAtomicGoalRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::DerivedTag>>, RoleAtomicGoalRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionRepository>,
    boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionRepository>,
    boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementRepository>,
    boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseRepository>,
    boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionRepository>,
    boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository>,
    boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository>,
    boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionRepository>,
    boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::StaticTag>>, BooleanAtomicStateRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::FluentTag>>, BooleanAtomicStateRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::DerivedTag>>, BooleanAtomicStateRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<ConceptTag>>, BooleanNonemptyRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<RoleTag>>, BooleanNonemptyRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<NumericalCountImpl<ConceptTag>>, NumericalCountRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<NumericalCountImpl<RoleTag>>, NumericalCountRepository<RoleTag>>,
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

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    Constructor<D> get_or_create(const std::string& sentence, const formalism::DomainImpl& domain);

    /* Concepts */
    Constructor<ConceptTag> get_or_create_concept_bot();
    Constructor<ConceptTag> get_or_create_concept_top();
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<ConceptTag> get_or_create_concept_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<ConceptTag> get_or_create_concept_atomic_goal(formalism::Predicate<P> predicate, bool polarity);
    Constructor<ConceptTag> get_or_create_concept_intersection(Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept);
    Constructor<ConceptTag> get_or_create_concept_union(Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept);
    Constructor<ConceptTag> get_or_create_concept_negation(Constructor<ConceptTag> concept_);
    Constructor<ConceptTag> get_or_create_concept_value_restriction(Constructor<RoleTag> role, Constructor<ConceptTag> concept_);
    Constructor<ConceptTag> get_or_create_concept_existential_quantification(Constructor<RoleTag> role, Constructor<ConceptTag> concept_);
    Constructor<ConceptTag> get_or_create_concept_role_value_map_containment(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);
    Constructor<ConceptTag> get_or_create_concept_role_value_map_equality(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);
    Constructor<ConceptTag> get_or_create_concept_nominal(formalism::Object object);

    /* Roles */
    Constructor<RoleTag> get_or_create_role_universal();
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<RoleTag> get_or_create_role_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<RoleTag> get_or_create_role_atomic_goal(formalism::Predicate<P> predicate, bool polarity);
    Constructor<RoleTag> get_or_create_role_intersection(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);
    Constructor<RoleTag> get_or_create_role_union(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);
    Constructor<RoleTag> get_or_create_role_complement(Constructor<RoleTag> role);
    Constructor<RoleTag> get_or_create_role_inverse(Constructor<RoleTag> role);
    Constructor<RoleTag> get_or_create_role_composition(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);
    Constructor<RoleTag> get_or_create_role_transitive_closure(Constructor<RoleTag> role);
    Constructor<RoleTag> get_or_create_role_reflexive_transitive_closure(Constructor<RoleTag> role);
    Constructor<RoleTag> get_or_create_role_restriction(Constructor<RoleTag> role, Constructor<ConceptTag> concept_);
    Constructor<RoleTag> get_or_create_role_identity(Constructor<ConceptTag> concept_);

    /* Booleans */
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<BooleanTag> get_or_create_boolean_atomic_state(formalism::Predicate<P> predicate);
    template<IsConceptOrRoleTag D>
    Constructor<BooleanTag> get_or_create_boolean_nonempty(Constructor<D> constructor);

    /* Numericals */
    template<IsConceptOrRoleTag D>
    Constructor<NumericalTag> get_or_create_numerical_count(Constructor<D> constructor);
    Constructor<NumericalTag>
    get_or_create_numerical_distance(Constructor<ConceptTag> left_concept, Constructor<RoleTag> role, Constructor<ConceptTag> right_concept);
};

}

#endif
