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

using HanaDerivationRules = boost::hana::map<
    boost::hana::pair<
        boost::hana::type<Concept>,
        boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, std::unordered_map<NonTerminal<Concept>, DerivationRuleSet<Concept, Primitive>>>,
                         boost::hana::pair<boost::hana::type<Composite>, std::unordered_map<NonTerminal<Concept>, DerivationRuleSet<Concept, Composite>>>>>,
    boost::hana::pair<
        boost::hana::type<Role>,
        boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, std::unordered_map<NonTerminal<Role>, DerivationRuleSet<Role, Primitive>>>,
                         boost::hana::pair<boost::hana::type<Composite>, std::unordered_map<NonTerminal<Role>, DerivationRuleSet<Role, Composite>>>>>>;

using HanaSubstitutionRules =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, SubstitutionRuleSet<Concept>>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, SubstitutionRuleSet<Role>>>>;

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
    HanaDerivationRules m_derivation_rules;

public:
    DerivationRulesContainer() = default;
    DerivationRulesContainer(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer& operator=(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer(DerivationRulesContainer&& other) = default;
    DerivationRulesContainer& operator=(DerivationRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<ConceptOrRole D, PrimitiveOrComposite C>
    auto insert(DerivationRule<D, C> rule)
    {
        return boost::hana::at_key(boost::hana::at_key(m_derivation_rules, boost::hana::type<D> {}), boost::hana::type<C> {})[rule->get_head()].insert(rule);
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D, PrimitiveOrComposite C>
    const std::unordered_map<NonTerminal<D>, DerivationRuleSet<D, C>>& get(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(boost::hana::at_key(m_derivation_rules, boost::hana::type<D> {}), boost::hana::type<C> {}).at(non_terminal);
    }

    HanaDerivationRules& get() { return m_derivation_rules; }
    const HanaDerivationRules& get() const { return m_derivation_rules; }
};

class SubstitutionRulesContainer
{
private:
    HanaSubstitutionRules m_substitution_rules;

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
    auto insert(SubstitutionRule<D> substitution_rule)
    {
        return boost::hana::at_key(m_substitution_rules, boost::hana::type<D> {})[substitution_rule->get_head()].insert(substitution_rule);
    }

    /**
     * Accessors
     */

    template<ConceptOrRole D>
    const std::unordered_map<NonTerminal<D>, SubstitutionRuleSet<D>>& get() const
    {
        return boost::hana::at_key(m_substitution_rules, boost::hana::type<D> {});
    }

    HanaSubstitutionRules& get() { return m_substitution_rules; }
    const HanaSubstitutionRules& get() const { return m_substitution_rules; }
};

}

#endif
