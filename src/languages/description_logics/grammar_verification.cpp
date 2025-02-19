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

#include "mimir/languages/description_logics/grammar_verification.hpp"

#include "mimir/languages/description_logics/grammar.hpp"

namespace mimir::dl::grammar
{

using NonTerminalSet = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_set<NonTerminal<Concept>>>,
                                        boost::hana::pair<boost::hana::type<Role>, std::unordered_set<NonTerminal<Role>>>>;

template<ConceptOrRole D>
static void parse_non_terminals_in_body(ConstructorOrNonTerminal<D> constructor, NonTerminalSet& ref_non_terminals)
{
}

static void parse_non_terminals_in_body(ConceptTop constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptBot constructor, NonTerminalSet& ref_non_terminals) {}
template<StaticOrFluentOrDerived P>
static void parse_non_terminals_in_body(ConceptAtomicState<P> constructor, NonTerminalSet& ref_non_terminals)
{
}
template<StaticOrFluentOrDerived P>
static void parse_non_terminals_in_body(ConceptAtomicGoal<P> constructor, NonTerminalSet& ref_non_terminals)
{
}
static void parse_non_terminals_in_body(ConceptIntersection constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptUnion constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptNegation constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptValueRestriction constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptExistentialQuantification constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptRoleValueMapContainment constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(ConceptRoleValueMapEquality constructor, NonTerminalSet& ref_non_terminals) {}

static void parse_non_terminals_in_body(RoleUniversalImpl constructor, NonTerminalSet& ref_non_terminals) {}
template<StaticOrFluentOrDerived P>
static void parse_non_terminals_in_body(RoleAtomicState<P> constructor, NonTerminalSet& ref_non_terminals)
{
}
template<StaticOrFluentOrDerived P>
static void parse_non_terminals_in_body(RoleAtomicGoal<P> constructor, NonTerminalSet& ref_non_terminals)
{
}
static void parse_non_terminals_in_body(RoleIntersection constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleUnion constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleComplement constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleInverse constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleComposition constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleTransitiveClosure constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleReflexiveTransitiveClosure constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleRestriction constructor, NonTerminalSet& ref_non_terminals) {}
static void parse_non_terminals_in_body(RoleIdentity constructor, NonTerminalSet& ref_non_terminals) {}

template<ConceptOrRole D>
void parse_non_terminals_in_body(ConstructorOrNonTerminal<D> constructor, NonTerminalSet& ref_non_terminals)
{
    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                // ref_non_terminals.insert(arg);
            }
            else if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                // TODO: parse recursively
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "parse_non_terminal(non_terminal, ref_non_terminals): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template<ConceptOrRole D>
static void parse_body_non_terminals(DerivationRule<D> rule, NonTerminalSet& ref_body_nonterminals)
{
    for (const auto& part : rule->get_constructor_or_non_terminals())
    {
        parse_non_terminals_in_body(part, ref_body_nonterminals);
    }
}

bool verify_grammar_is_well_defined(const Grammar& grammar)
{
    auto head_nonterminals = NonTerminalSet {};
    auto body_nonterminals = NonTerminalSet {};

    boost::hana::for_each(grammar.get_rules(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              auto value = boost::hana::second(pair);

                              for (const auto& rules_entry : value)
                              {
                                  const auto head_nonterminal = rules_entry.first;

                                  boost::hana::at_key(head_nonterminals, key).insert(head_nonterminal);

                                  for (const auto& rule : rules_entry.second)
                                  {
                                      parse_body_non_terminals(rule, body_nonterminals);
                                  }
                              }
                          });
}
}
