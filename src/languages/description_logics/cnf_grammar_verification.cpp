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

#include "cnf_grammar_verification.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"

namespace mimir::dl::cnf_grammar
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
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        constructor->get_body()->accept(this->m_constructor_visitor);
    }

    CollectHeadAndBodyNonTerminalsDerivationRuleVisitor(RecurseNonTerminalVisitor<D>& nonterminal_visitor,
                                                        RecurseConstructorVisitor<D>& constructor_visitor,
                                                        std::unordered_set<std::string>& head_non_terminals) :
        RecurseDerivationRuleVisitor<D>(nonterminal_visitor, constructor_visitor),
        m_head_non_terminals(head_non_terminals)
    {
    }
};

template<FeatureCategory D>
class CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor : public RecurseSubstitutionRuleVisitor<D>
{
private:
    std::unordered_set<std::string>& m_head_non_terminals;
    std::unordered_set<std::string>& m_body_non_terminals;

public:
    void visit(SubstitutionRule<D> constructor) override
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        m_body_non_terminals.insert(constructor->get_body()->get_name());
    }

    CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor(RecurseNonTerminalVisitor<D>& nonterminal_visitor,
                                                          std::unordered_set<std::string>& head_non_terminals,
                                                          std::unordered_set<std::string>& body_non_terminals) :
        RecurseSubstitutionRuleVisitor<D>(nonterminal_visitor),
        m_head_non_terminals(head_non_terminals),
        m_body_non_terminals(body_non_terminals)
    {
    }
};

template<FeatureCategory D>
class CollectHeadAndBodyNonTerminalsNonTerminalVisitor : public RecurseNonTerminalVisitor<D>
{
private:
    std::unordered_set<std::string>& m_body_non_terminals;

public:
    void visit(NonTerminal<D> constructor) override { m_body_non_terminals.insert(constructor->get_name()); }

    explicit CollectHeadAndBodyNonTerminalsNonTerminalVisitor(std::unordered_set<std::string>& body_non_terminals) : m_body_non_terminals(body_non_terminals) {}
};

extern std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> collect_head_and_body_nonterminals(const Grammar& grammar)
{
    auto head_nonterminals = std::unordered_set<std::string> {};
    auto body_nonterminals = std::unordered_set<std::string> {};

    auto body_concept_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Concept>(body_nonterminals);
    auto body_role_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Role>(body_nonterminals);
    auto body_boolean_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Boolean>(body_nonterminals);
    auto body_numerical_nonterminal_visitor = CollectHeadAndBodyNonTerminalsNonTerminalVisitor<Numerical>(body_nonterminals);

    auto body_concept_and_role_nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<RecurseNonTerminalVisitor<Concept>&>(body_concept_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<RecurseNonTerminalVisitor<Role>&>(body_role_nonterminal_visitor))));

    auto body_nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<RecurseNonTerminalVisitor<Concept>&>(body_concept_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<RecurseNonTerminalVisitor<Role>&>(body_role_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(static_cast<RecurseNonTerminalVisitor<Boolean>&>(body_boolean_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Numerical> {},
                               std::ref(static_cast<RecurseNonTerminalVisitor<Numerical>&>(body_numerical_nonterminal_visitor))));

    auto concept_nonterminal_visitor = RecurseNonTerminalVisitor<Concept>();
    auto role_nonterminal_visitor = RecurseNonTerminalVisitor<Role>();
    auto boolean_nonterminal_visitor = RecurseNonTerminalVisitor<Boolean>();
    auto numerical_nonterminal_visitor = RecurseNonTerminalVisitor<Numerical>();

    auto concept_visitor = RecurseConstructorVisitor<Concept>(body_concept_and_role_nonterminal_visitors);
    auto role_visitor = RecurseConstructorVisitor<Role>(body_concept_and_role_nonterminal_visitors);
    auto boolean_visitor = RecurseConstructorVisitor<Boolean>(body_concept_and_role_nonterminal_visitors);
    auto numerical_visitor = RecurseConstructorVisitor<Numerical>(body_concept_and_role_nonterminal_visitors);

    auto concept_derivation_rule_visitor =
        CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Concept>(concept_nonterminal_visitor, concept_visitor, head_nonterminals);
    auto role_derivation_rule_visitor = CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Role>(role_nonterminal_visitor, role_visitor, head_nonterminals);
    auto boolean_derivation_rule_visitor =
        CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Boolean>(boolean_nonterminal_visitor, boolean_visitor, head_nonterminals);
    auto numerical_derivation_rule_visitor =
        CollectHeadAndBodyNonTerminalsDerivationRuleVisitor<Numerical>(numerical_nonterminal_visitor, numerical_visitor, head_nonterminals);

    auto derivation_rule_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<RecurseDerivationRuleVisitor<Concept>&>(concept_derivation_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<RecurseDerivationRuleVisitor<Role>&>(role_derivation_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(static_cast<RecurseDerivationRuleVisitor<Boolean>&>(boolean_derivation_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Numerical> {},
                               std::ref(static_cast<RecurseDerivationRuleVisitor<Numerical>&>(numerical_derivation_rule_visitor))));

    auto concept_substitution_rule_visitor =
        CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor<Concept>(body_concept_nonterminal_visitor, head_nonterminals, body_nonterminals);
    auto role_substitution_rule_visitor =
        CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor<Role>(body_role_nonterminal_visitor, head_nonterminals, body_nonterminals);
    auto boolean_substitution_rule_visitor =
        CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor<Boolean>(body_boolean_nonterminal_visitor, head_nonterminals, body_nonterminals);
    auto numerical_substitution_rule_visitor =
        CollectHeadAndBodyNonTerminalsSubstitutionRuleVisitor<Numerical>(body_numerical_nonterminal_visitor, head_nonterminals, body_nonterminals);

    auto substitution_rule_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {},
                               std::ref(static_cast<RecurseSubstitutionRuleVisitor<Concept>&>(concept_substitution_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<RecurseSubstitutionRuleVisitor<Role>&>(role_substitution_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Boolean> {},
                               std::ref(static_cast<RecurseSubstitutionRuleVisitor<Boolean>&>(boolean_substitution_rule_visitor))),
        boost::hana::make_pair(boost::hana::type<Numerical> {},
                               std::ref(static_cast<RecurseSubstitutionRuleVisitor<Numerical>&>(numerical_substitution_rule_visitor))));

    auto grammar_visitor = RecurseGrammarVisitor(body_nonterminal_visitors, derivation_rule_visitors, substitution_rule_visitors);

    grammar.accept(grammar_visitor);

    return { head_nonterminals, body_nonterminals };
}

void verify_grammar_is_well_defined(const Grammar& grammar)
{
    auto [head_nonterminals, body_nonterminals] = collect_head_and_body_nonterminals(grammar);

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
