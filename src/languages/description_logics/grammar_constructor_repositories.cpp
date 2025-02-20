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

template<ConceptOrRole D>
DerivationRule<D> DLConstructorRepositories::get_or_create_concept_derivation_rule(NonTerminal<D> non_terminal,
                                                                                   ConstructorOrNonTerminalList<D> constructor_or_non_terminals)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<DerivationRuleImpl<D>> {})
        .get_or_create(non_terminal, std::move(constructor_or_non_terminals));
}

template DerivationRule<Concept>
DLConstructorRepositories::get_or_create_concept_derivation_rule(NonTerminal<Concept> non_terminal,
                                                                 ConstructorOrNonTerminalList<Concept> constructor_or_non_terminals);
template DerivationRule<Role> DLConstructorRepositories::get_or_create_concept_derivation_rule(NonTerminal<Role> non_terminal,
                                                                                               ConstructorOrNonTerminalList<Role> constructor_or_non_terminals);

template<ConceptOrRole D>
NonTerminal<D> DLConstructorRepositories::get_or_create_concept_nonterminal(std::string name)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(std::move(name));
}

template NonTerminal<Concept> DLConstructorRepositories::get_or_create_concept_nonterminal(std::string name);
template NonTerminal<Role> DLConstructorRepositories::get_or_create_concept_nonterminal(std::string name);

template<ConceptOrRole D>
ConstructorOrNonTerminal<D> DLConstructorRepositories::get_or_create_concept_constructor_or_nonterminal(std::variant<Constructor<D>, NonTerminal<D>> choice)
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConstructorOrNonTerminalImpl<D>> {}).get_or_create(std::move(choice));
}

template ConstructorOrNonTerminal<Concept>
DLConstructorRepositories::get_or_create_concept_constructor_or_nonterminal(std::variant<Constructor<Concept>, NonTerminal<Concept>> choice);
template ConstructorOrNonTerminal<Role>
DLConstructorRepositories::get_or_create_concept_constructor_or_nonterminal(std::variant<Constructor<Role>, NonTerminal<Role>> choice);

/* Concepts */
ConceptBot DLConstructorRepositories::get_or_create_concept_bot()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();
}

ConceptTop DLConstructorRepositories::get_or_create_concept_top() {}

template<StaticOrFluentOrDerived P>
ConceptAtomicState<P> DLConstructorRepositories::get_or_create_concept_atomic_state(Predicate<P> predicate)
{
}

template<StaticOrFluentOrDerived P>
ConceptAtomicGoal<P> DLConstructorRepositories::get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated)
{
}

ConceptIntersection DLConstructorRepositories::get_or_create_concept_intersection(ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                                                                  ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right)
{
}

ConceptUnion DLConstructorRepositories::get_or_create_concept_union(ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                                                    ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right)
{
}

ConceptNegation DLConstructorRepositories::get_or_create_concept_negation(ConstructorOrNonTerminal<Concept> concept_or_non_terminal) {}

ConceptValueRestriction DLConstructorRepositories::get_or_create_concept_value_restriction(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                                           ConstructorOrNonTerminal<Concept> concept_or_non_terminal)
{
}

ConceptExistentialQuantification
DLConstructorRepositories::get_or_create_concept_existential_quantification(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                            ConstructorOrNonTerminal<Concept> concept_or_non_terminal)
{
}

ConceptRoleValueMapContainment
DLConstructorRepositories::get_or_create_concept_role_value_map_containment(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                            ConstructorOrNonTerminal<Role> role_or_non_terminal_right)
{
}

ConceptRoleValueMapEquality DLConstructorRepositories::get_or_create_concept_role_value_map_equality(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                                                     ConstructorOrNonTerminal<Role> role_or_non_terminal_right)
{
}

ConceptNominal DLConstructorRepositories::get_or_create_concept_nominal(Object object) {}

/* Roles */

RoleUniversal DLConstructorRepositories::get_or_create_role_universal() {}

template<StaticOrFluentOrDerived P>
RoleAtomicState<P> DLConstructorRepositories::get_or_create_role_atomic_state(Predicate<P> predicate)
{
}

template<StaticOrFluentOrDerived P>
RoleAtomicGoal<P> DLConstructorRepositories::get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated)
{
}

RoleIntersection DLConstructorRepositories::get_or_create_role_intersection(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                            ConstructorOrNonTerminal<Role> role_or_non_terminal_right)
{
}

RoleUnion DLConstructorRepositories::get_or_create_role_union(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                              ConstructorOrNonTerminal<Role> role_or_non_terminal_right)
{
}

RoleComplement DLConstructorRepositories::get_or_create_role_complement(ConstructorOrNonTerminal<Role> role_or_non_terminal) {}

RoleInverse DLConstructorRepositories::get_or_create_role_inverse(ConstructorOrNonTerminal<Role> role_or_non_terminal) {}

RoleComposition DLConstructorRepositories::get_or_create_role_composition(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                          ConstructorOrNonTerminal<Role> role_or_non_terminal_right)
{
}

RoleTransitiveClosure DLConstructorRepositories::get_or_create_role_transitive_closure(ConstructorOrNonTerminal<Role> role_or_non_terminal) {}

RoleReflexiveTransitiveClosure DLConstructorRepositories::get_or_create_role_reflexive_transitive_closure(ConstructorOrNonTerminal<Role> role_or_non_terminal)
{
}

RoleRestriction DLConstructorRepositories::get_or_create_role_restriction(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                          ConstructorOrNonTerminal<Concept> concept_or_non_terminal)
{
}

RoleIdentity DLConstructorRepositories::get_or_create_role_identity(ConstructorOrNonTerminal<Concept> concept_or_non_terminal) {}

}
