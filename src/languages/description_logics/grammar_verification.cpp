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
#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

namespace mimir::dl::grammar
{

using NonTerminalSet = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_set<NonTerminal<Concept>>>,
                                        boost::hana::pair<boost::hana::type<Role>, std::unordered_set<NonTerminal<Role>>>>;

class CollectNonTerminalsVisitor : public Visitor
{
private:
    NonTerminalSet& m_head_non_terminals;
    NonTerminalSet& m_body_non_terminals;

public:
    /* Concepts */
    void visit(DerivationRule<Concept> constructor) {}
    void visit(NonTerminal<Concept> constructor) {}
    void visit(ConstructorOrNonTerminal<Concept> constructor) {}
    void visit(ConceptBot constructor) {}
    void visit(ConceptTop constructor) {}
    void visit(ConceptAtomicState<Static> constructor) {}
    void visit(ConceptAtomicState<Fluent> constructor) {}
    void visit(ConceptAtomicState<Derived> constructor) {}
    void visit(ConceptAtomicGoal<Static> constructor) {}
    void visit(ConceptAtomicGoal<Fluent> constructor) {}
    void visit(ConceptAtomicGoal<Derived> constructor) {}
    void visit(ConceptIntersection constructor) {}
    void visit(ConceptUnion constructor) {}
    void visit(ConceptNegation constructor) {}
    void visit(ConceptValueRestriction constructor) {}
    void visit(ConceptExistentialQuantification constructor) {}
    void visit(ConceptRoleValueMapContainment constructor) {}
    void visit(ConceptRoleValueMapEquality constructor) {}
    void visit(ConceptNominal constructor) {}

    /* Roles */
    void visit(DerivationRule<Role> constructor) {}
    void visit(NonTerminal<Role> constructor) {}
    void visit(ConstructorOrNonTerminal<Role> constructor) {}
    void visit(RoleUniversal constructor) {}
    void visit(RoleAtomicState<Static> constructor) {}
    void visit(RoleAtomicState<Fluent> constructor) {}
    void visit(RoleAtomicState<Derived> constructor) {}
    void visit(RoleAtomicGoal<Static> constructor) {}
    void visit(RoleAtomicGoal<Fluent> constructor) {}
    void visit(RoleAtomicGoal<Derived> constructor) {}
    void visit(RoleIntersection constructor) {}
    void visit(RoleUnion constructor) {}
    void visit(RoleComplement constructor) {}
    void visit(RoleInverse constructor) {}
    void visit(RoleComposition constructor) {}
    void visit(RoleTransitiveClosure constructor) {}
    void visit(RoleReflexiveTransitiveClosure constructor) {}
    void visit(RoleRestriction constructor) {}
    void visit(RoleIdentity constructor) {}

    CollectNonTerminalsVisitor(NonTerminalSet& head_non_terminals, NonTerminalSet& body_non_terminals) :
        m_head_non_terminals(head_non_terminals),
        m_body_non_terminals(body_non_terminals)
    {
    }
};

bool verify_grammar_is_well_defined(const Grammar& grammar)
{
    auto head_nonterminals = NonTerminalSet {};
    auto body_nonterminals = NonTerminalSet {};

    auto visitor = CollectNonTerminalsVisitor(head_nonterminals, body_nonterminals);

    boost::hana::for_each(grammar.get_rules(),
                          [&](auto&& pair)
                          {
                              auto value = boost::hana::second(pair);

                              for (const auto& rules_entry : value)
                              {
                                  for (const auto& rule : rules_entry.second)
                                  {
                                      visitor.visit(rule);
                                  }
                              }
                          });
}
}
