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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"

#include <loki/loki.hpp>

namespace mimir::dl::grammar
{

using StartNonterminals = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::optional<NonTerminal<Concept>>>,
                                           boost::hana::pair<boost::hana::type<Role>, std::optional<NonTerminal<Role>>>>;

using PerNonterminalGrammarRules =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, DerivationRuleSet<Concept>>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, DerivationRuleSet<Role>>>>;

class GrammarRules
{
private:
    StartNonterminals m_start_nonterminals;
    PerNonterminalGrammarRules m_rules;

public:
    GrammarRules() : m_start_nonterminals(), m_rules() {}
    explicit GrammarRules(PerNonterminalGrammarRules rules, StartNonterminals start_nonterminals) :
        m_start_nonterminals(std::move(start_nonterminals)),
        m_rules(std::move(rules))
    {
    }

    template<ConceptOrRole D>
    const std::optional<NonTerminal<D>>& get_start_nonterminal() const
    {
        return boost::hana::at_key(m_start_nonterminals, boost::hana::type<D> {});
    }

    template<ConceptOrRole D>
    const DerivationRuleSet<D>& get_rules(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(m_rules, boost::hana::type<D> {}).at(non_terminal);
    }
};

class Grammar
{
private:
    /* Memory */
    GrammarConstructorRepositories m_grammar_constructor_repos;

    /* The rules of the grammar. */
    GrammarRules m_rules;

public:
    /// @brief Create a grammar from a BNF description.
    Grammar(std::string bnf_description, Domain domain);

    /// @brief Tests whether a dl constructor satisfies the grammar specification.
    /// @param constructor is the dl constructor to test.
    /// @return true iff the dl constructor satisfies the grammar specification, and false otherwise.
    template<ConceptOrRole D>
    bool test_match(dl::Constructor<D> constructor) const;

    /**
     * Getters
     */

    template<ConceptOrRole D>
    const DerivationRuleList<D>& get_primitive_production_rules() const;

    template<ConceptOrRole D>
    const DerivationRuleList<D>& get_composite_production_rules() const;

    template<ConceptOrRole D>
    const DerivationRuleList<D>& get_alternative_rules() const;
};
}

#endif
