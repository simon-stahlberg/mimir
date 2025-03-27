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

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

HanaRepositories& Repositories::get_repositories() { return m_repositories; }

const HanaRepositories& Repositories::get_repositories() const { return m_repositories; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
NonTerminal<D> Repositories::get_or_create_nonterminal(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(std::move(name));
}

template NonTerminal<ConceptTag> Repositories::get_or_create_nonterminal(std::string name);
template NonTerminal<RoleTag> Repositories::get_or_create_nonterminal(std::string name);
template NonTerminal<BooleanTag> Repositories::get_or_create_nonterminal(std::string name);
template NonTerminal<NumericalTag> Repositories::get_or_create_nonterminal(std::string name);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
DerivationRule<D> Repositories::get_or_create_derivation_rule(NonTerminal<D> head, Constructor<D> body)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DerivationRuleImpl<D>> {}).get_or_create(head, body);
}

template DerivationRule<ConceptTag> Repositories::get_or_create_derivation_rule(NonTerminal<ConceptTag> head, Constructor<ConceptTag> body);
template DerivationRule<RoleTag> Repositories::get_or_create_derivation_rule(NonTerminal<RoleTag> head, Constructor<RoleTag> body);
template DerivationRule<BooleanTag> Repositories::get_or_create_derivation_rule(NonTerminal<BooleanTag> head, Constructor<BooleanTag> body);
template DerivationRule<NumericalTag> Repositories::get_or_create_derivation_rule(NonTerminal<NumericalTag> head, Constructor<NumericalTag> body);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
SubstitutionRule<D> Repositories::get_or_create_substitution_rule(NonTerminal<D> head, NonTerminal<D> body)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<SubstitutionRuleImpl<D>> {}).get_or_create(head, body);
}

template SubstitutionRule<ConceptTag> Repositories::get_or_create_substitution_rule(NonTerminal<ConceptTag> head, NonTerminal<ConceptTag> body);
template SubstitutionRule<RoleTag> Repositories::get_or_create_substitution_rule(NonTerminal<RoleTag> head, NonTerminal<RoleTag> body);
template SubstitutionRule<BooleanTag> Repositories::get_or_create_substitution_rule(NonTerminal<BooleanTag> head, NonTerminal<BooleanTag> body);
template SubstitutionRule<NumericalTag> Repositories::get_or_create_substitution_rule(NonTerminal<NumericalTag> head, NonTerminal<NumericalTag> body);

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

Constructor<ConceptTag> Repositories::get_or_create_concept_intersection(NonTerminal<ConceptTag> left_nonterminal_concept,
                                                                         NonTerminal<ConceptTag> right_nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {})
        .get_or_create(left_nonterminal_concept, right_nonterminal_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_union(NonTerminal<ConceptTag> left_nonterminal_concept,
                                                                  NonTerminal<ConceptTag> right_nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_nonterminal_concept, right_nonterminal_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_negation(NonTerminal<ConceptTag> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(nonterminal_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_value_restriction(NonTerminal<RoleTag> nonterminal_role,
                                                                              NonTerminal<ConceptTag> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_existential_quantification(NonTerminal<RoleTag> nonterminal_role,
                                                                                       NonTerminal<ConceptTag> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_containment(NonTerminal<RoleTag> left_nonterminal_role,
                                                                                       NonTerminal<RoleTag> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_equality(NonTerminal<RoleTag> left_nonterminal_role,
                                                                                    NonTerminal<RoleTag> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .get_or_create(left_nonterminal_role, right_nonterminal_role);
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

Constructor<RoleTag> Repositories::get_or_create_role_intersection(NonTerminal<RoleTag> left_nonterminal_role, NonTerminal<RoleTag> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_union(NonTerminal<RoleTag> left_nonterminal_role, NonTerminal<RoleTag> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_complement(NonTerminal<RoleTag> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_inverse(NonTerminal<RoleTag> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_composition(NonTerminal<RoleTag> left_nonterminal_role, NonTerminal<RoleTag> right_nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_nonterminal_role, right_nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_transitive_closure(NonTerminal<RoleTag> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_reflexive_transitive_closure(NonTerminal<RoleTag> nonterminal_role)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(nonterminal_role);
}

Constructor<RoleTag> Repositories::get_or_create_role_restriction(NonTerminal<RoleTag> nonterminal_role, NonTerminal<ConceptTag> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(nonterminal_role, nonterminal_concept);
}

Constructor<RoleTag> Repositories::get_or_create_role_identity(NonTerminal<ConceptTag> nonterminal_concept)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(nonterminal_concept);
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
Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(NonTerminal<D> constructor_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanNonemptyImpl<D>> {}).get_or_create(constructor_nonterminal);
}

template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(NonTerminal<ConceptTag> constructor_nonterminal);
template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(NonTerminal<RoleTag> constructor_nonterminal);

/* Numericals */

template<IsConceptOrRoleTag D>
Constructor<NumericalTag> Repositories::get_or_create_numerical_count(NonTerminal<D> constructor_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalCountImpl<D>> {}).get_or_create(constructor_nonterminal);
}

template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(NonTerminal<ConceptTag> constructor_nonterminal);
template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(NonTerminal<RoleTag> constructor_nonterminal);

Constructor<NumericalTag> Repositories::get_or_create_numerical_distance(NonTerminal<ConceptTag> left_concept_nonterminal,
                                                                         NonTerminal<RoleTag> role_nonterminal,
                                                                         NonTerminal<ConceptTag> right_concept_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalDistanceImpl> {})
        .get_or_create(left_concept_nonterminal, role_nonterminal, right_concept_nonterminal);
}
}
