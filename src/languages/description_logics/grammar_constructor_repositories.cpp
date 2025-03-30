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

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

HanaRepositories& Repositories::get_repositories() { return m_repositories; }

const HanaRepositories& Repositories::get_repositories() const { return m_repositories; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
DerivationRule<D> Repositories::get_or_create_derivation_rule(NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DerivationRuleImpl<D>> {})
        .get_or_create(non_terminal, std::move(constructor_or_non_terminals));
}

template DerivationRule<ConceptTag> Repositories::get_or_create_derivation_rule(NonTerminal<ConceptTag> non_terminal,
                                                                                ConstructorOrNonTerminalList<ConceptTag> constructor_or_non_terminals);
template DerivationRule<RoleTag> Repositories::get_or_create_derivation_rule(NonTerminal<RoleTag> non_terminal,
                                                                             ConstructorOrNonTerminalList<RoleTag> constructor_or_non_terminals);
template DerivationRule<BooleanTag> Repositories::get_or_create_derivation_rule(NonTerminal<BooleanTag> non_terminal,
                                                                                ConstructorOrNonTerminalList<BooleanTag> constructor_or_non_terminals);
template DerivationRule<NumericalTag> Repositories::get_or_create_derivation_rule(NonTerminal<NumericalTag> non_terminal,
                                                                                  ConstructorOrNonTerminalList<NumericalTag> constructor_or_non_terminals);

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
ConstructorOrNonTerminal<D> Repositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<D>, NonTerminal<D>> choice)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConstructorOrNonTerminalImpl<D>> {}).get_or_create(std::move(choice));
}

template ConstructorOrNonTerminal<ConceptTag>
Repositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<ConceptTag>, NonTerminal<ConceptTag>> choice);
template ConstructorOrNonTerminal<RoleTag>
Repositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<RoleTag>, NonTerminal<RoleTag>> choice);
template ConstructorOrNonTerminal<BooleanTag>
Repositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<BooleanTag>, NonTerminal<BooleanTag>> choice);
template ConstructorOrNonTerminal<NumericalTag>
Repositories::get_or_create_constructor_or_nonterminal(std::variant<Constructor<NumericalTag>, NonTerminal<NumericalTag>> choice);

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

Constructor<ConceptTag> Repositories::get_or_create_concept_intersection(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                                         ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptIntersectionImpl> {})
        .get_or_create(left_concept_or_nonterminal, right_concept_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_union(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                                  ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptUnionImpl> {}).get_or_create(left_concept_or_nonterminal, right_concept_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_negation(ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptNegationImpl> {}).get_or_create(concept_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_value_restriction(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                              ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptValueRestrictionImpl> {}).get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_existential_quantification(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                                       ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptExistentialQuantificationImpl> {})
        .get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_containment(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                                       ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapContainmentImpl> {})
        .get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

Constructor<ConceptTag> Repositories::get_or_create_concept_role_value_map_equality(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                                    ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptRoleValueMapEqualityImpl> {})
        .get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
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

Constructor<RoleTag> Repositories::get_or_create_role_intersection(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                   ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIntersectionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_union(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                            ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleUnionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_complement(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleComplementImpl> {}).get_or_create(role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_inverse(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleInverseImpl> {}).get_or_create(role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_composition(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                  ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleCompositionImpl> {}).get_or_create(left_role_or_nonterminal, right_role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_transitive_closure(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleTransitiveClosureImpl> {}).get_or_create(role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_reflexive_transitive_closure(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleReflexiveTransitiveClosureImpl> {}).get_or_create(role_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_restriction(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                  ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleRestrictionImpl> {}).get_or_create(role_or_nonterminal, concept_or_nonterminal);
}

Constructor<RoleTag> Repositories::get_or_create_role_identity(ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<RoleIdentityImpl> {}).get_or_create(concept_or_nonterminal);
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
Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(ConstructorOrNonTerminal<D> constructor_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<BooleanNonemptyImpl<D>> {}).get_or_create(constructor_or_nonterminal);
}

template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(ConstructorOrNonTerminal<ConceptTag> constructor_or_nonterminal);
template Constructor<BooleanTag> Repositories::get_or_create_boolean_nonempty(ConstructorOrNonTerminal<RoleTag> constructor_or_nonterminal);

/* Numericals */

template<IsConceptOrRoleTag D>
Constructor<NumericalTag> Repositories::get_or_create_numerical_count(ConstructorOrNonTerminal<D> constructor_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalCountImpl<D>> {}).get_or_create(constructor_or_nonterminal);
}

template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(ConstructorOrNonTerminal<ConceptTag> constructor_or_nonterminal);
template Constructor<NumericalTag> Repositories::get_or_create_numerical_count(ConstructorOrNonTerminal<RoleTag> constructor_or_nonterminal);

Constructor<NumericalTag> Repositories::get_or_create_numerical_distance(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                                         ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                         ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NumericalDistanceImpl> {})
        .get_or_create(left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal);
}

}
