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

HanaConstructorRepositories& ConstructorRepositories::get_repositories() { return m_repositories; }

const HanaConstructorRepositories& ConstructorRepositories::get_repositories() const { return m_repositories; }

template<FeatureCategory D>
DerivationRule<D> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<D> non_terminal,
                                                                         ConstructorOrNonTerminalList<D> constructor_or_non_terminals)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DerivationRuleImpl<D>> {})
        .get_or_create(non_terminal, std::move(constructor_or_non_terminals));
}

template DerivationRule<Concept> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Concept> non_terminal,
                                                                                        ConstructorOrNonTerminalList<Concept> constructor_or_non_terminals);
template DerivationRule<Role> ConstructorRepositories::get_or_create_derivation_rule(NonTerminal<Role> non_terminal,
                                                                                     ConstructorOrNonTerminalList<Role> constructor_or_non_terminals);

template<FeatureCategory D>
NonTerminal<D> ConstructorRepositories::get_or_create_nonterminal(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(std::move(name));
}

template NonTerminal<Concept> ConstructorRepositories::get_or_create_nonterminal(std::string name);
template NonTerminal<Role> ConstructorRepositories::get_or_create_nonterminal(std::string name);

template<FeatureCategory D>
ConstructorOrNonTerminal<D> ConstructorRepositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<D>, NonTerminal<D>> choice)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConstructorOrNonTerminalImpl<D>> {}).get_or_create(std::move(choice));
}

template ConstructorOrNonTerminal<Concept>
ConstructorRepositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<Concept>, NonTerminal<Concept>> choice);
template ConstructorOrNonTerminal<Role>
ConstructorRepositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<Role>, NonTerminal<Role>> choice);

/* Concepts */
ConceptBot ConstructorRepositories::get_or_create_concept_bot()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

ConceptTop ConstructorRepositories::get_or_create_concept_top()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptTopImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
ConceptAtomicState<P> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template ConceptAtomicState<Static> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Static> predicate);
template ConceptAtomicState<Fluent> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Fluent> predicate);
template ConceptAtomicState<Derived> ConstructorRepositories::get_or_create_concept_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
ConceptAtomicGoal<P> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template ConceptAtomicGoal<Static> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Static> predicate, bool is_negated);
template ConceptAtomicGoal<Fluent> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template ConceptAtomicGoal<Derived> ConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<Derived> predicate, bool is_negated);

ConceptIntersection ConstructorRepositories::get_or_create_concept_intersection(ConstructorOrNonTerminal<Concept> left_concept_or_nonterminal,
                                                                                ConstructorOrNonTerminal<Concept> right_concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {})
        .get_or_create(left_concept_or_nonterminal, right_concept_or_nonterminal);
}

ConceptUnion ConstructorRepositories::get_or_create_concept_union(ConstructorOrNonTerminal<Concept> left_concept_or_nonterminal,
                                                                  ConstructorOrNonTerminal<Concept> right_concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_concept_or_nonterminal, right_concept_or_nonterminal);
}

ConceptNegation ConstructorRepositories::get_or_create_concept_negation(ConstructorOrNonTerminal<Concept> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(concept_or_nonterminal);
}

ConceptValueRestriction ConstructorRepositories::get_or_create_concept_value_restriction(ConstructorOrNonTerminal<Role> role_or_nonterminal,
                                                                                         ConstructorOrNonTerminal<Concept> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

ConceptExistentialQuantification
ConstructorRepositories::get_or_create_concept_existential_quantification(ConstructorOrNonTerminal<Role> role_or_nonterminal,
                                                                          ConstructorOrNonTerminal<Concept> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {})
        .get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

ConceptRoleValueMapContainment
ConstructorRepositories::get_or_create_concept_role_value_map_containment(ConstructorOrNonTerminal<Role> left_role_or_nonterminal,
                                                                          ConstructorOrNonTerminal<Role> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

ConceptRoleValueMapEquality ConstructorRepositories::get_or_create_concept_role_value_map_equality(ConstructorOrNonTerminal<Role> left_role_or_nonterminal,
                                                                                                   ConstructorOrNonTerminal<Role> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

ConceptNominal ConstructorRepositories::get_or_create_concept_nominal(Object object)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNominalImpl> {}).get_or_create(object);
}

/* Roles */

RoleUniversal ConstructorRepositories::get_or_create_role_universal()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUniversalImpl> {}).get_or_create();
}

template<StaticOrFluentOrDerived P>
RoleAtomicState<P> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<P> predicate)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicStateImpl<P>> {}).get_or_create(predicate);
}

template RoleAtomicState<Static> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Static> predicate);
template RoleAtomicState<Fluent> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Fluent> predicate);
template RoleAtomicState<Derived> ConstructorRepositories::get_or_create_role_atomic_state(Predicate<Derived> predicate);

template<StaticOrFluentOrDerived P>
RoleAtomicGoal<P> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, is_negated);
}

template RoleAtomicGoal<Static> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Static> predicate, bool is_negated);
template RoleAtomicGoal<Fluent> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Fluent> predicate, bool is_negated);
template RoleAtomicGoal<Derived> ConstructorRepositories::get_or_create_role_atomic_goal(Predicate<Derived> predicate, bool is_negated);

RoleIntersection ConstructorRepositories::get_or_create_role_intersection(ConstructorOrNonTerminal<Role> left_role_or_nonterminal,
                                                                          ConstructorOrNonTerminal<Role> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

RoleUnion ConstructorRepositories::get_or_create_role_union(ConstructorOrNonTerminal<Role> left_role_or_nonterminal,
                                                            ConstructorOrNonTerminal<Role> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

RoleComplement ConstructorRepositories::get_or_create_role_complement(ConstructorOrNonTerminal<Role> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(role_or_nonterminal);
}

RoleInverse ConstructorRepositories::get_or_create_role_inverse(ConstructorOrNonTerminal<Role> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(role_or_nonterminal);
}

RoleComposition ConstructorRepositories::get_or_create_role_composition(ConstructorOrNonTerminal<Role> left_role_or_nonterminal,
                                                                        ConstructorOrNonTerminal<Role> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

RoleTransitiveClosure ConstructorRepositories::get_or_create_role_transitive_closure(ConstructorOrNonTerminal<Role> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(role_or_nonterminal);
}

RoleReflexiveTransitiveClosure ConstructorRepositories::get_or_create_role_reflexive_transitive_closure(ConstructorOrNonTerminal<Role> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(role_or_nonterminal);
}

RoleRestriction ConstructorRepositories::get_or_create_role_restriction(ConstructorOrNonTerminal<Role> role_or_nonterminal,
                                                                        ConstructorOrNonTerminal<Concept> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

RoleIdentity ConstructorRepositories::get_or_create_role_identity(ConstructorOrNonTerminal<Concept> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(concept_or_nonterminal);
}

}
