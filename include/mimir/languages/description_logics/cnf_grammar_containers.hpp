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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONTAINERS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONTAINERS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"

#include <loki/loki.hpp>
#include <optional>
#include <unordered_map>
#include <utility>

namespace mimir::dl::cnf_grammar
{
using HanaStartSymbols = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::optional<NonTerminal<Concept>>>,
                                          boost::hana::pair<boost::hana::type<Role>, std::optional<NonTerminal<Role>>>>;

using DerivationRulesMap =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, DerivationRuleList<Concept>>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, DerivationRuleList<Role>>>>;

using SubstitutionRulesMap =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, SubstitutionRuleList<Concept>>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, SubstitutionRuleList<Role>>>>;

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

    HanaStartSymbols& get() { return m_symbols; }
    const HanaStartSymbols& get() const { return m_symbols; }
};

class DerivationRulesContainer
{
private:
    DerivationRulesMap m_map;
    DerivationRuleLists<Concept, Role> m_lists;

    static const DerivationRuleLists<Concept, Role> empty_lists;

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
    void push_back(DerivationRule<D> rule)
    {
        boost::hana::at_key(m_map, boost::hana::type<D> {})[rule->get_head()].push_back(rule);
        boost::hana::at_key(m_lists, boost::hana::type<D> {}).push_back(rule);
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D>
    const DerivationRuleList<D>& get(NonTerminal<D> non_terminal) const
    {
        const auto& container = boost::hana::at_key(m_map, boost::hana::type<D> {});

        auto it = container.find(non_terminal);
        if (it == container.end())
        {
            return boost::hana::at_key(empty_lists, boost::hana::type<D> {});
        }

        return it->second;
    }

    template<ConceptOrRole D>
    const DerivationRuleList<D>& get() const
    {
        return boost::hana::at_key(m_lists, boost::hana::type<D> {});
    }

    DerivationRuleLists<Concept, Role>& get() { return m_lists; }
    const DerivationRuleLists<Concept, Role>& get() const { return m_lists; }
};

class SubstitutionRulesContainer
{
private:
    SubstitutionRulesMap m_map;
    SubstitutionRuleLists<Concept, Role> m_lists;

    static const SubstitutionRuleLists<Concept, Role> empty_lists;

public:
    SubstitutionRulesContainer() = default;
    SubstitutionRulesContainer(const SubstitutionRulesContainer& other) = delete;
    SubstitutionRulesContainer& operator=(const SubstitutionRulesContainer& other) = delete;
    SubstitutionRulesContainer(SubstitutionRulesContainer&& other) = default;
    SubstitutionRulesContainer& operator=(SubstitutionRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<ConceptOrRole D>
    void push_back(SubstitutionRule<D> rule)
    {
        boost::hana::at_key(m_map, boost::hana::type<D> {})[rule->get_head()].push_back(rule);
        boost::hana::at_key(m_lists, boost::hana::type<D> {}).push_back(rule);
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D>
    const SubstitutionRuleList<D>& get(NonTerminal<D> non_terminal) const
    {
        const auto& container = boost::hana::at_key(m_map, boost::hana::type<D> {});

        auto it = container.find(non_terminal);
        if (it == container.end())
        {
            return boost::hana::at_key(empty_lists, boost::hana::type<D> {});
        }

        return it->second;
    }

    template<ConceptOrRole D>
    const SubstitutionRuleList<D>& get() const
    {
        return boost::hana::at_key(m_lists, boost::hana::type<D> {});
    }

    SubstitutionRuleLists<Concept, Role>& get() { return m_lists; }
    const SubstitutionRuleLists<Concept, Role>& get() const { return m_lists; }
};

}

#endif
