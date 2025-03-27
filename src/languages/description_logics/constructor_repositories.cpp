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

#include "mimir/languages/description_logics/constructor_repositories.hpp"

#include "sentence_parser.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{

HanaRepositories& Repositories::get_repositories() { return m_repositories; }

const HanaRepositories& Repositories::get_repositories() const { return m_repositories; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Constructor<D> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain)
{
    return parse_sentence<D>(sentence, domain, *this);
}

template Constructor<ConceptTag> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<RoleTag> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<BooleanTag> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<NumericalTag> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);

/* Concepts */
Constructor<ConceptTag> Repositories::get_or_create_concept_bot()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

Constructor<ConceptTag> Repositories::get_or_create_concept_top()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptTopImpl> {}).get_or_create();
}

template<IsStaticOrFluentOrDerivedTag P>
Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_state(Predicate<StaticTag> predicate);
template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_state(Predicate<FluentTag> predicate);
template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_state(Predicate<DerivedTag> predicate);

template<IsStaticOrFluentOrDerivedTag P>
Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_goal(Predicate<P> predicate, bool polarity)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, polarity);
}

template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_goal(Predicate<StaticTag> predicate, bool polarity);
template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_goal(Predicate<FluentTag> predicate, bool polarity);
template Constructor<ConceptTag> Repositories::get_or_create_concept_atomic_goal(Predicate<DerivedTag> predicate, bool polarity);

Constructor<ConceptTag> Repositories::get_or_create_concept_intersection(Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {}).get_or_create(left_concept, right_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_union(Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_concept, right_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_negation(Constructor<ConceptTag> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(concept_);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_value_restriction(Constructor<RoleTag> role, Constructor<ConceptTag> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(role, concept_);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_existential_quantification(Constructor<RoleTag> role, Constructor<ConceptTag> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {}).get_or_create(role, concept_);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_containment(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {}).get_or_create(left_role, right_role);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_equality(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {}).get_or_create(left_role, right_role);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_nominal(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNominalImpl> {}).get_or_create(object);
}

/* Roles */

Constructor<RoleTag> Repositories::get_or_create_role_universal()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUniversalImpl> {}).get_or_create();
}

template<IsStaticOrFluentOrDerivedTag P>
Constructor<RoleTag> Repositories::get_or_create_role_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<RoleTag> Repositories::get_or_create_role_atomic_state(Predicate<StaticTag> predicate);
template Constructor<RoleTag> Repositories::get_or_create_role_atomic_state(Predicate<FluentTag> predicate);
template Constructor<RoleTag> Repositories::get_or_create_role_atomic_state(Predicate<DerivedTag> predicate);

template<IsStaticOrFluentOrDerivedTag P>
Constructor<RoleTag> Repositories::get_or_create_role_atomic_goal(Predicate<P> predicate, bool polarity)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, polarity);
}

template Constructor<RoleTag> Repositories::get_or_create_role_atomic_goal(Predicate<StaticTag> predicate, bool polarity);
template Constructor<RoleTag> Repositories::get_or_create_role_atomic_goal(Predicate<FluentTag> predicate, bool polarity);
template Constructor<RoleTag> Repositories::get_or_create_role_atomic_goal(Predicate<DerivedTag> predicate, bool polarity);

Constructor<RoleTag> Repositories::get_or_create_role_intersection(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_union(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_complement(Constructor<RoleTag> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(role);
}

Constructor<RoleTag> Repositories::get_or_create_role_inverse(Constructor<RoleTag> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(role);
}

Constructor<RoleTag> Repositories::get_or_create_role_composition(Constructor<RoleTag> left_role, Constructor<RoleTag> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_transitive_closure(Constructor<RoleTag> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(role);
}

Constructor<RoleTag> Repositories::get_or_create_role_reflexive_transitive_closure(Constructor<RoleTag> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(role);
}

Constructor<RoleTag> Repositories::get_or_create_role_restriction(Constructor<RoleTag> role, Constructor<ConceptTag> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(role, concept_);
}

Constructor<RoleTag> Repositories::get_or_create_role_identity(Constructor<ConceptTag> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(concept_);
}

/* Booleans */

template<IsStaticOrFluentOrDerivedTag P>
Constructor<BooleanTag> Repositories::get_or_create_boolean_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<BooleanTag> Repositories::get_or_create_boolean_atomic_state(Predicate<StaticTag> predicate);
template Constructor<BooleanTag> Repositories::get_or_create_boolean_atomic_state(Predicate<FluentTag> predicate);
template Constructor<BooleanTag> Repositories::get_or_create_boolean_atomic_state(Predicate<DerivedTag> predicate);

template<IsConceptOrRoleTag D>
Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(Constructor<D> constructor)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanNonemptyImpl<D>> {}).get_or_create(constructor);
}

template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(Constructor<ConceptTag> constructor);
template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(Constructor<RoleTag> constructor);

/* Numericals */

template<IsConceptOrRoleTag D>
Constructor<NumericalTag> Repositories::get_or_create_numerical_count(Constructor<D> constructor)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalCountImpl<D>> {}).get_or_create(constructor);
}

template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(Constructor<ConceptTag> constructor);
template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(Constructor<RoleTag> constructor);

Constructor<NumericalTag>
Repositories::get_or_create_numerical_distance(Constructor<ConceptTag> left_concept, Constructor<RoleTag> role, Constructor<ConceptTag> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalDistanceImpl> {}).get_or_create(left_concept, role, right_concept);
}

}
