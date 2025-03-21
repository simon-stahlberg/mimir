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

template<FeatureCategory D>
Constructor<D> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain)
{
    return parse_sentence<D>(sentence, domain, *this);
}

template Constructor<Concept> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<Role> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<Boolean> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);
template Constructor<Numerical> Repositories::get_or_create(const std::string& sentence, const DomainImpl& domain);

/* Concepts */
Constructor<Concept> Repositories::get_or_create_concept_bot()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

Constructor<Concept> Repositories::get_or_create_concept_top()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptTopImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
Constructor<Concept> Repositories::get_or_create_concept_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Concept> Repositories::get_or_create_concept_atomic_state(Predicate<Static> predicate);
template Constructor<Concept> Repositories::get_or_create_concept_atomic_state(Predicate<Fluent> predicate);
template Constructor<Concept> Repositories::get_or_create_concept_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
Constructor<Concept> Repositories::get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template Constructor<Concept> Repositories::get_or_create_concept_atomic_goal(Predicate<Static> predicate, bool is_negated);
template Constructor<Concept> Repositories::get_or_create_concept_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template Constructor<Concept> Repositories::get_or_create_concept_atomic_goal(Predicate<Derived> predicate, bool is_negated);

Constructor<Concept> Repositories::get_or_create_concept_intersection(Constructor<Concept> left_concept, Constructor<Concept> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {}).get_or_create(left_concept, right_concept);
}

Constructor<Concept> Repositories::get_or_create_concept_union(Constructor<Concept> left_concept, Constructor<Concept> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_concept, right_concept);
}

Constructor<Concept> Repositories::get_or_create_concept_negation(Constructor<Concept> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(concept_);
}

Constructor<Concept> Repositories::get_or_create_concept_value_restriction(Constructor<Role> role, Constructor<Concept> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(role, concept_);
}

Constructor<Concept> Repositories::get_or_create_concept_existential_quantification(Constructor<Role> role, Constructor<Concept> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {}).get_or_create(role, concept_);
}

Constructor<Concept> Repositories::get_or_create_concept_role_value_map_containment(Constructor<Role> left_role, Constructor<Role> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {}).get_or_create(left_role, right_role);
}

Constructor<Concept> Repositories::get_or_create_concept_role_value_map_equality(Constructor<Role> left_role, Constructor<Role> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {}).get_or_create(left_role, right_role);
}

Constructor<Concept> Repositories::get_or_create_concept_nominal(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNominalImpl> {}).get_or_create(object);
}

/* Roles */

Constructor<Role> Repositories::get_or_create_role_universal()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUniversalImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
Constructor<Role> Repositories::get_or_create_role_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Role> Repositories::get_or_create_role_atomic_state(Predicate<Static> predicate);
template Constructor<Role> Repositories::get_or_create_role_atomic_state(Predicate<Fluent> predicate);
template Constructor<Role> Repositories::get_or_create_role_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
Constructor<Role> Repositories::get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template Constructor<Role> Repositories::get_or_create_role_atomic_goal(Predicate<Static> predicate, bool is_negated);
template Constructor<Role> Repositories::get_or_create_role_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template Constructor<Role> Repositories::get_or_create_role_atomic_goal(Predicate<Derived> predicate, bool is_negated);

Constructor<Role> Repositories::get_or_create_role_intersection(Constructor<Role> left_role, Constructor<Role> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<Role> Repositories::get_or_create_role_union(Constructor<Role> left_role, Constructor<Role> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<Role> Repositories::get_or_create_role_complement(Constructor<Role> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(role);
}

Constructor<Role> Repositories::get_or_create_role_inverse(Constructor<Role> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(role);
}

Constructor<Role> Repositories::get_or_create_role_composition(Constructor<Role> left_role, Constructor<Role> right_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_role, right_role);
}

Constructor<Role> Repositories::get_or_create_role_transitive_closure(Constructor<Role> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(role);
}

Constructor<Role> Repositories::get_or_create_role_reflexive_transitive_closure(Constructor<Role> role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(role);
}

Constructor<Role> Repositories::get_or_create_role_restriction(Constructor<Role> role, Constructor<Concept> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(role, concept_);
}

Constructor<Role> Repositories::get_or_create_role_identity(Constructor<Concept> concept_)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(concept_);
}

/* Booleans */

template<StaticOrFluentOrDerived P>
Constructor<Boolean> Repositories::get_or_create_boolean_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Boolean> Repositories::get_or_create_boolean_atomic_state(Predicate<Static> predicate);
template Constructor<Boolean> Repositories::get_or_create_boolean_atomic_state(Predicate<Fluent> predicate);
template Constructor<Boolean> Repositories::get_or_create_boolean_atomic_state(Predicate<Derived> predicate);

template<DescriptionLogicCategory D>
Constructor<Boolean> Repositories::get_or_create_boolean_nonempty(Constructor<D> constructor)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanNonemptyImpl<D>> {}).get_or_create(constructor);
}

template Constructor<Boolean> Repositories::get_or_create_boolean_nonempty(Constructor<Concept> constructor);
template Constructor<Boolean> Repositories::get_or_create_boolean_nonempty(Constructor<Role> constructor);

/* Numericals */

template<DescriptionLogicCategory D>
Constructor<Numerical> Repositories::get_or_create_numerical_count(Constructor<D> constructor)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalCountImpl<D>> {}).get_or_create(constructor);
}

template Constructor<Numerical> Repositories::get_or_create_numerical_count(Constructor<Concept> constructor);
template Constructor<Numerical> Repositories::get_or_create_numerical_count(Constructor<Role> constructor);

Constructor<Numerical>
Repositories::get_or_create_numerical_distance(Constructor<Concept> left_concept, Constructor<Role> role, Constructor<Concept> right_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalDistanceImpl> {}).get_or_create(left_concept, role, right_concept);
}

}
