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
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"

#include <loki/loki.hpp>

namespace mimir::dl::grammar
{
using StartSymbols = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::optional<NonTerminal<Concept>>>,
                                      boost::hana::pair<boost::hana::type<Role>, std::optional<NonTerminal<Role>>>>;

using GrammarRules = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, DerivationRuleSet<Concept>>>,
                                      boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, DerivationRuleSet<Role>>>>;

enum class GrammarSpecificationEnum
{
    FRANCES_ET_AL_AAAI2021 = 0,
};

class Grammar
{
private:
    /* Memory */
    ConstructorRepositories m_repositories;

    /* The rules of the grammar. */
    StartSymbols m_start_symbols;
    GrammarRules m_rules;

public:
    Grammar(ConstructorRepositories repositories, StartSymbols start_symbols, GrammarRules rules);

    /// @brief Create a grammar from a BNF description for a given domain.
    /// The domain is used for error checking only to ensure that predicates or constants are available.
    Grammar(std::string bnf_description, Domain domain);

    /// @brief Create a predefined grammar for a given domain.
    Grammar(GrammarSpecificationEnum type, Domain domain);

    /// @brief Tests whether a dl constructor satisfies the grammar specification.
    /// @param constructor is the dl constructor to test.
    /// @return true iff the dl constructor satisfies the grammar specification, and false otherwise.
    template<ConceptOrRole D>
    bool test_match(dl::Constructor<D> constructor) const;

    /**
     * Getters
     */

    template<ConceptOrRole D>
    const std::optional<NonTerminal<D>>& get_start_symbol() const
    {
        return boost::hana::at_key(m_start_symbols, boost::hana::type<D> {});
    }

    const StartSymbols& get_start_symbols() const { return m_start_symbols; }

    template<ConceptOrRole D>
    const DerivationRuleSet<D>& get_rules(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(m_rules, boost::hana::type<D> {}).at(non_terminal);
    }

    const GrammarRules& get_rules() const { return m_rules; }
};
}

#endif
