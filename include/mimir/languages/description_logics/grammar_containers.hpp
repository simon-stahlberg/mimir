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

class StartSymbolsContainer
{
private:
    HanaStartSymbols m_symbols;

public:
    StartSymbolsContainer() = default;
    StartSymbolsContainer(const StartSymbolsContainer& other) = default;
    StartSymbolsContainer& operator=(const StartSymbolsContainer& other) = default;
    StartSymbolsContainer(StartSymbolsContainer&& other) = default;
    StartSymbolsContainer& operator=(StartSymbolsContainer&& other) = default;

    /**
     * Modifiers
     */

    template<FeatureCategory D>
    auto insert(NonTerminal<D> start_symbol)
    {
        assert(boost::hana::at_key(m_symbols, boost::hana::type<D> {}) == std::nullopt);

        boost::hana::at_key(m_symbols, boost::hana::type<D> {}) = start_symbol;
    }

    /**
     * Accessors
     */

    template<FeatureCategory D>
    const std::optional<NonTerminal<D>>& get() const
    {
        return boost::hana::at_key(m_symbols, boost::hana::type<D> {});
    }

    const HanaStartSymbols& get() const { return m_symbols; }
};

class DerivationRulesContainer
{
private:
    NonTerminalMap<DerivationRuleSet, Concept, Role> m_map;
    DerivationRuleSets<Concept, Role> m_lists;

public:
    DerivationRulesContainer() = default;
    DerivationRulesContainer(const DerivationRulesContainer& other) = default;
    DerivationRulesContainer& operator=(const DerivationRulesContainer& other) = default;
    DerivationRulesContainer(DerivationRulesContainer&& other) = default;
    DerivationRulesContainer& operator=(DerivationRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<FeatureCategory D>
    void insert(DerivationRule<D> rule)
    {
        boost::hana::at_key(m_map, boost::hana::type<D> {})[rule->get_non_terminal()].insert(rule);
        boost::hana::at_key(m_lists, boost::hana::type<D> {}).insert(rule);
    }

    /**
     * Accessors
     */

    template<FeatureCategory D>
    const DerivationRuleSet<D>& at(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(m_map, boost::hana::type<D> {}).at(non_terminal);
    }

    const DerivationRuleSets<Concept, Role>& get() const { return m_lists; }
};

}

#endif
