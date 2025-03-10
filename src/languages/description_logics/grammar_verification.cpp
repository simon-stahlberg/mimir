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

#include "mimir/common/printers.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

namespace mimir::dl::grammar
{

template<FeatureCategory D>
class CollectHeadAndBodyNonTerminalsDerivationRuleVisitor : public RecurseDerivationRuleVisitor<D>
{
private:
    std::unordered_set<std::string>& m_head_non_terminals;

public:
    void visit(DerivationRule<D> constructor) override
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_non_terminal()->get_name());

        for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
        {
            constructor_or_nonterminal->accept(*this->m_constructor_or_nonterminal_visitor);
        }
    }

    CollectHeadAndBodyNonTerminalsDerivationRuleVisitor(std::unordered_set<std::string>& head_non_terminals) : m_head_non_terminals(head_non_terminals) {}
};

template<FeatureCategory D>
class CollectHeadAndBodyNonTerminalsNonTerminalVisitor : public RecurseNonTerminalVisitor<D>
{
private:
    std::unordered_set<std::string>& m_body_non_terminals;

public:
    void visit(NonTerminal<D> constructor) override { m_body_non_terminals.insert(constructor->get_name()); }

    CollectHeadAndBodyNonTerminalsNonTerminalVisitor(std::unordered_set<std::string>& body_non_terminals) : m_body_non_terminals(body_non_terminals) {}
};

void verify_grammar_is_well_defined(const Grammar& grammar)
{
    std::cout << grammar << std::endl;

    auto head_nonterminals = std::unordered_set<std::string> {};
    auto body_nonterminals = std::unordered_set<std::string> {};

    auto concept_visitor = RecurseConstructorVisitor<Concept>();
    auto role_visitor = RecurseConstructorVisitor<Role>();
    auto boolean_visitor = RecurseConstructorVisitor<Boolean>();
    auto numerical_visitor = RecurseConstructorVisitor<Numerical>();
    auto concept_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Concept>();
    auto role_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Role>();
    auto boolean_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Boolean>();
    auto numerical_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Numerical>();
    auto concept_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Concept>(body_nonterminals);
    auto role_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Role>(body_nonterminals);
    auto boolean_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Boolean>(body_nonterminals);
    auto numerical_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Numerical>(body_nonterminals);
    auto concept_derivation_rule_visitor = CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Concept>(head_nonterminals);
    auto role_derivation_rule_visitor = CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Role>(head_nonterminals);
    auto boolean_derivation_rule_visitor = CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Boolean>(head_nonterminals);
    auto numerical_derivation_rule_visitor = CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Numerical>(head_nonterminals);

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_or_nonterminal_visitor),
                                                                       boost::hana::make_pair(boost::hana::type<Role> {}, &role_or_nonterminal_visitor));

    concept_visitor.initialize(concept_and_role_nonterminal_visitors);
    role_visitor.initialize(concept_and_role_nonterminal_visitors);
    boolean_visitor.initialize(concept_and_role_nonterminal_visitors);
    numerical_visitor.initialize(concept_and_role_nonterminal_visitors);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_visitor);

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto value = boost::hana::second(pair);

                              for (const auto& rule : value)
                              {
                                  using T = std::decay_t<decltype(rule)>;
                                  if constexpr (std::is_same_v<T, DerivationRule<Concept>>)
                                  {
                                      rule->accept(concept_derivation_rule_visitor);
                                  }
                                  else if constexpr (std::is_same_v<T, DerivationRule<Role>>)
                                  {
                                      rule->accept(role_derivation_rule_visitor);
                                  }
                                  else if constexpr (std::is_same_v<T, DerivationRule<Boolean>>)
                                  {
                                      rule->accept(boolean_derivation_rule_visitor);
                                  }
                                  else if constexpr (std::is_same_v<T, DerivationRule<Numerical>>)
                                  {
                                      rule->accept(numerical_derivation_rule_visitor);
                                  }
                                  else
                                  {
                                      static_assert(dependent_false<T>::value,
                                                    "verify_grammar_is_well_defined(grammar): Missing implementation for DerivationRule type.");
                                  }
                              }
                          });

    // std::cout << body_nonterminals << std::endl;
    // std::cout << head_nonterminals << std::endl;

    /* 1. Verify that all body non-terminals appear in a head. */
    for (const auto& body_nonterminal : body_nonterminals)
    {
        if (!head_nonterminals.contains(body_nonterminal))
        {
            throw std::runtime_error("verify_grammar_is_well_defined(grammar): The body nonterminal " + body_nonterminal + " is never defined in a rule head.");
        }
    }

    /* 2. Verify that grammar has a start symbol. */
    if (!boost::hana::any_of(boost::hana::values(grammar.get_start_symbols_container().get()), [](auto&& value) { return value.has_value(); }))
    {
        throw std::runtime_error("verify_grammar_is_well_defined(grammar): The grammar does not define any start symbol.");
    }

    /* 3. Verify that all start symbols appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && !head_nonterminals.contains(start_non_terminal.value()->get_name()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " is never defined in a rule head.");
                              }
                          });

    /* 4. Verify that all start symbols only appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && body_nonterminals.contains(start_non_terminal.value()->get_name()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " should not appear in a rule body.");
                              }
                          });
}
}
