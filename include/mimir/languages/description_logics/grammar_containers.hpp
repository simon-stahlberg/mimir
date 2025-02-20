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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONTAINERS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONTAINERS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"

#include <loki/loki.hpp>

namespace mimir::dl::grammar
{
using HanaStartSymbols = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::optional<NonTerminal<Concept>>>,
                                          boost::hana::pair<boost::hana::type<Role>, std::optional<NonTerminal<Role>>>>;

using HanaDerivationRules =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, DerivationRuleSet<Concept>>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, DerivationRuleSet<Role>>>>;

class StartSymbolsContainer
{
private:
    HanaStartSymbols m_symbols;

public:
    StartSymbolsContainer() = default;
    StartSymbolsContainer(const StartSymbolsContainer& other) = delete;
    StartSymbolsContainer& operator=(const StartSymbolsContainer& other) = delete;
    StartSymbolsContainer(StartSymbolsContainer&& other) = default;
    StartSymbolsContainer& operator=(StartSymbolsContainer&& other) = default;

    /**
     * Modifiers
     */

    template<ConceptOrRole D>
    auto insert(NonTerminal<D> start_symbol)
    {
        assert(boost::hana::at_key(m_symbols, boost::hana::type<D> {}) == std::nullopt);

        boost::hana::at_key(m_symbols, boost::hana::type<D> {}) = start_symbol;
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D>
    const std::optional<NonTerminal<D>>& get() const
    {
        return boost::hana::at_key(m_symbols, boost::hana::type<D> {});
    }

    const HanaStartSymbols& get() const { return m_symbols; }
};

class DerivationRulesContainer
{
private:
    HanaDerivationRules m_rules;

public:
    DerivationRulesContainer() = default;
    DerivationRulesContainer(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer& operator=(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer(DerivationRulesContainer&& other) = default;
    DerivationRulesContainer& operator=(DerivationRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<ConceptOrRole D>
    auto insert(DerivationRule<D> rule)
    {
        return boost::hana::at_key(m_rules, boost::hana::type<D> {})[rule->get_non_terminal()].insert(rule);
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D>
    const DerivationRuleSet<D>& get(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(m_rules, boost::hana::type<D> {}).at(non_terminal);
    }

    const HanaDerivationRules& get() const { return m_rules; }
};
}

#endif
