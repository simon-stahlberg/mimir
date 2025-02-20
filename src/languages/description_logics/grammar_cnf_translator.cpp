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

#include "mimir/languages/description_logics/grammar_cnf_translator.hpp"

#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

namespace mimir::dl::grammar
{

Grammar translate_to_cnf(const Grammar& grammar)
{
    auto start_symbols = StartSymbolsContainer();
    auto grammar_rules = DerivationRulesContainer();
    // auto repositories = GrammarConstructorRepositories();

    /* Step 1: flatten A ::= B | C  ==> A ::= B and A::= C */

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& non_terminal_and_rules : second)
                              {
                                  const auto& [non_terminal, rules] = non_terminal_and_rules;

                                  for (const auto& rule : rules)
                                  {
                                      for (const auto& choice : rule->get_constructor_or_non_terminals())
                                      {
                                          //  boost::hana::at_key(grammar_rules, key).
                                      }
                                  }
                              }
                          });
    {
    }
}
}
