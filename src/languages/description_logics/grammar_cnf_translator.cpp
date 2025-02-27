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

#include "grammar_cnf_translator.hpp"

#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

namespace mimir::dl::grammar
{

/**
 * Step 1: Collect all non terminals to assign novel names to newly generated ones.
 */

template<ConceptOrRole D>
class CollectNonTerminalsNonTerminalVisitor : public RecurseNonTerminalVisitor<D>
{
private:
    NonTerminalMap<std::string>& m_nonterminal_map;

public:
    CollectNonTerminalsNonTerminalVisitor(NonTerminalMap<std::string>& nonterminal_map) : m_nonterminal_map(nonterminal_map) {}

    void visit(NonTerminal<D> constructor) override { m_nonterminal_map.insert(constructor->get_name(), constructor); }
};

static NonTerminalMap<std::string> collect_nonterminals_from_grammar(const Grammar& grammar)
{
    auto nonterminal_map = NonTerminalMap<std::string>();

    auto concept_visitor = RecurseConstructorVisitor<Concept>();
    auto role_visitor = RecurseConstructorVisitor<Role>();
    auto concept_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Concept>();
    auto role_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Role>();
    auto concept_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Concept>(nonterminal_map);
    auto role_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Role>(nonterminal_map);
    auto concept_derivation_rule_visitor = RecurseDerivationRuleVisitor<Concept>();
    auto role_derivation_rule_visitor = RecurseDerivationRuleVisitor<Role>();
    auto concept_start_symbol_visitor = RecurseNonTerminalVisitor<Concept>();
    auto role_start_symbol_visitor = RecurseNonTerminalVisitor<Role>();
    auto grammar_visitor = RecurseGrammarVisitor();

    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor, role_start_symbol_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    grammar.accept(grammar_visitor);

    return nonterminal_map;
}

cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar)
{
    /* Collect all non-terminals to assign new names during translation */
    auto nonterminal_map = collect_nonterminals_from_grammar(grammar);

    // auto repositories = GrammarConstructorRepositories();

    /* Step 1: flatten A ::= B | C  ==> A ::= B and A::= C */
}
}
