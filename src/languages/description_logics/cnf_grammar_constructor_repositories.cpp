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

#include "mimir/languages/description_logics/cnf_grammar_constructor_repositories.hpp"

namespace mimir::dl::cnf_grammar
{

HanaConstructorRepositories& ConstructorRepositories::get_repositories() { return m_repositories; }

const HanaConstructorRepositories& ConstructorRepositories::get_repositories() const { return m_repositories; }

template<FeatureCategory D>
NonTerminal<D> ConstructorRepositories::get_or_create_nonterminal(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(std::move(name));
}

template NonTerminal<Concept> ConstructorRepositories::get_or_create_nonterminal(std::string name);
template NonTerminal<Role> ConstructorRepositories::get_or_create_nonterminal(std::string name);
template NonTerminal<Boolean> ConstructorRepositories::get_or_create_nonterminal(std::string name);
template NonTerminal<Numerical> ConstructorRepositories::get_or_create_nonterminal(std::string name);

template<FeatureCategory D>
DerivationRule<D> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<D> head, Constructor<D> body)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DerivationRuleImpl<D>> {}).get_or_create(head, body);
}

template DerivationRule<Concept> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Concept> head, Constructor<Concept> body);
template DerivationRule<Role> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Role> head, Constructor<Role> body);
template DerivationRule<Boolean> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Boolean> head, Constructor<Boolean> body);
template DerivationRule<Numerical> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Numerical> head, Constructor<Numerical> body);

template<FeatureCategory D>
SubstitutionRule<D> ConstructorRepositories::get_or_create_substitution_rule(NonTerminal<D> head, NonTerminal<D> body)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<SubstitutionRuleImpl<D>> {}).get_or_create(head, body);
}

template SubstitutionRule<Concept> ConstructorRepositories::get_or_create_substitution_rule(NonTerminal<Concept> head, NonTerminal<Concept> body);
template SubstitutionRule<Role> ConstructorRepositories::get_or_create_substitution_rule(NonTerminal<Role> head, NonTerminal<Role> body);
template SubstitutionRule<Boolean> ConstructorRepositories::get_or_create_substitution_rule(NonTerminal<Boolean> head, NonTerminal<Boolean> body);
template SubstitutionRule<Numerical> ConstructorRepositories::get_or_create_substitution_rule(NonTerminal<Numerical> head, NonTerminal<Numerical> body);

/* Concepts */
Constructor<Concept> ConstructorRepositories::get_or_create_concept_bot()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_top()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptTopImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Static> predicate);
template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Fluent> predicate);
template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Static> predicate, bool is_negated);
template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template Constructor<Concept> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Derived> predicate, bool is_negated);

Constructor<Concept> ConstructorRepositories::get_or_create_concept_intersection(NonTerminal<Concept> left_nonterminal_concept,
                                                                                 NonTerminal<Concept> right_nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {})
        .get_or_create(left_nonterminal_concept, right_nonterminal_concept);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_union(NonTerminal<Concept> left_nonterminal_concept,
                                                                          NonTerminal<Concept> right_nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_nonterminal_concept, right_nonterminal_concept);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_negation(NonTerminal<Concept> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(nonterminal_concept);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_value_restriction(NonTerminal<Role> nonterminal_role,
                                                                                      NonTerminal<Concept> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_existential_quantification(NonTerminal<Role> nonterminal_role,
                                                                                               NonTerminal<Concept> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_role_value_map_containment(NonTerminal<Role> left_nonterminal_role,
                                                                                               NonTerminal<Role> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_role_value_map_equality(NonTerminal<Role> left_nonterminal_role,
                                                                                            NonTerminal<Role> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<Concept> ConstructorRepositories::get_or_create_concept_nominal(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNominalImpl> {}).get_or_create(object);
}

/* Roles */

Constructor<Role> ConstructorRepositories::get_or_create_role_universal()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUniversalImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Static> predicate);
template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Fluent> predicate);
template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Static> predicate, bool is_negated);
template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template Constructor<Role> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Derived> predicate, bool is_negated);

Constructor<Role> ConstructorRepositories::get_or_create_role_intersection(NonTerminal<Role> left_nonterminal_role, NonTerminal<Role> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_union(NonTerminal<Role> left_nonterminal_role, NonTerminal<Role> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_complement(NonTerminal<Role> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_inverse(NonTerminal<Role> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_composition(NonTerminal<Role> left_nonterminal_role, NonTerminal<Role> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_transitive_closure(NonTerminal<Role> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_reflexive_transitive_closure(NonTerminal<Role> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(nonterminal_role);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_restriction(NonTerminal<Role> nonterminal_role, NonTerminal<Concept> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<Role> ConstructorRepositories::get_or_create_role_identity(NonTerminal<Concept> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(nonterminal_concept);
}

/* Booleans */

template<StaticOrFluentOrDerived P>
Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_atomic_state(Predicate<Static> predicate);
template Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_atomic_state(Predicate<Fluent> predicate);
template Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_atomic_state(Predicate<Derived> predicate);

template<DescriptionLogicCategory D>
Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_nonempty(NonTerminal<D> constructor_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanNonemptyImpl<D>> {}).get_or_create(constructor_nonterminal);
}

template Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_nonempty(NonTerminal<Concept> constructor_nonterminal);
template Constructor<Boolean> ConstructorRepositories::get_or_create_boolean_nonempty(NonTerminal<Role> constructor_nonterminal);

/* Numericals */

template<DescriptionLogicCategory D>
Constructor<Numerical> ConstructorRepositories::get_or_create_numerical_count(NonTerminal<D> constructor_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalCountImpl<D>> {}).get_or_create(constructor_nonterminal);
}

template Constructor<Numerical> ConstructorRepositories::get_or_create_numerical_count(NonTerminal<Concept> constructor_nonterminal);
template Constructor<Numerical> ConstructorRepositories::get_or_create_numerical_count(NonTerminal<Role> constructor_nonterminal);

Constructor<Numerical> ConstructorRepositories::get_or_create_numerical_distance(NonTerminal<Concept> left_concept_nonterminal,
                                                                                 NonTerminal<Role> role_nonterminal,
                                                                                 NonTerminal<Concept> right_concept_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalDistanceImpl> {})
        .get_or_create(left_concept_nonterminal, role_nonterminal, right_concept_nonterminal);
}
}
