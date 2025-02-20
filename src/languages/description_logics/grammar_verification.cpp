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

#include "grammar_constructor_visitor_interface_impl.hpp"
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
    void visit(DerivationRule<Concept> constructor) override
    {
        // Store the head nonterminal but do not visit the head non terminal.
        boost::hana::at_key(m_head_non_terminals, boost::hana::type<Concept> {}).insert(constructor->get_non_terminal());
        // Recurse further
        Visitor::visit(constructor->get_constructor_or_non_terminals());
    }
    void visit(NonTerminal<Concept> constructor) override { boost::hana::at_key(m_body_non_terminals, boost::hana::type<Concept> {}).insert(constructor); }

    /* Roles */
    void visit(DerivationRule<Role> constructor) override
    {
        // Store the head nonterminal but do not visit the head non terminal.
        boost::hana::at_key(m_head_non_terminals, boost::hana::type<Role> {}).insert(constructor->get_non_terminal());
        // Recurse further
        Visitor::visit(constructor->get_constructor_or_non_terminals());
    }
    void visit(NonTerminal<Role> constructor) override { boost::hana::at_key(m_body_non_terminals, boost::hana::type<Role> {}).insert(constructor); }

    CollectNonTerminalsVisitor(NonTerminalSet& head_non_terminals, NonTerminalSet& body_non_terminals) :
        m_head_non_terminals(head_non_terminals),
        m_body_non_terminals(body_non_terminals)
    {
    }
};

void verify_grammar_is_well_defined(const Grammar& grammar)
{
    auto head_nonterminals = NonTerminalSet {};
    auto body_nonterminals = NonTerminalSet {};

    auto visitor = CollectNonTerminalsVisitor(head_nonterminals, body_nonterminals);

    boost::hana::for_each(grammar.get_rules_container().get(),
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

    /* 1. Verify that all body non-terminals appear in a head. */
    boost::hana::for_each(body_nonterminals,
                          [&](auto&& pair)
                          {
                              auto& key = boost::hana::first(pair);
                              auto& non_terminals = boost::hana::second(pair);

                              for (const auto& body_nonterminal : non_terminals)
                              {
                                  if (!boost::hana::at_key(head_nonterminals, key).contains(body_nonterminal))
                                  {
                                      throw std::runtime_error("verify_grammar_is_well_defined(grammar): The body nonterminal " + body_nonterminal->get_name()
                                                               + " is never defined in a rule head.");
                                  }
                              }
                          });

    /* 2. Verify that grammar has a start symbol. */
    if (!boost::hana::any_of(boost::hana::values(grammar.get_start_symbols_container().get()), [](auto&& value) { return value.has_value(); }))
    {
        throw std::runtime_error("verify_grammar_is_well_defined(grammar): The grammar does not define any start symbol.");
    }

    /* 3. Verify that all start symbols appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& key = boost::hana::first(pair);
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && !boost::hana::at_key(head_nonterminals, key).contains(start_non_terminal.value()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " is never defined in a rule head.");
                              }
                          });

    /* 4. Verify that all start symbols only appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& key = boost::hana::first(pair);
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && boost::hana::at_key(body_nonterminals, key).contains(start_non_terminal.value()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " should not appear in a rule body.");
                              }
                          });
}
}
