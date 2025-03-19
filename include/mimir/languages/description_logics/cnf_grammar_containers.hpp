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

namespace mimir::languages::dl::cnf_grammar
{

class DerivationRulesContainer
{
private:
    NonTerminalMap<DerivationRuleList, Concept, Role, Boolean, Numerical> m_map;
    DerivationRuleLists<Concept, Role, Boolean, Numerical> m_lists;

    static const DerivationRuleLists<Concept, Role, Boolean, Numerical> empty_lists;

public:
    DerivationRulesContainer() = default;
    DerivationRulesContainer(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer& operator=(const DerivationRulesContainer& other) = delete;
    DerivationRulesContainer(DerivationRulesContainer&& other) = default;
    DerivationRulesContainer& operator=(DerivationRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<FeatureCategory D>
    void push_back(DerivationRule<D> rule)
    {
        boost::hana::at_key(m_map, boost::hana::type<D> {})[rule->get_head()].push_back(rule);
        boost::hana::at_key(m_lists, boost::hana::type<D> {}).push_back(rule);
    }

    /**
     * Accessors
     */

    template<FeatureCategory D>
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

    template<FeatureCategory D>
    const DerivationRuleList<D>& get() const
    {
        return boost::hana::at_key(m_lists, boost::hana::type<D> {});
    }

    const DerivationRuleLists<Concept, Role, Boolean, Numerical>& get() const { return m_lists; }

    size_t size() const
    {
        return boost::hana::fold(m_lists,
                                 std::size_t { 0 },
                                 [](std::size_t acc, auto pair)
                                 {
                                     return acc + boost::hana::second(pair).size();  // Sum up sizes
                                 });
    }
};

class SubstitutionRulesContainer
{
private:
    NonTerminalMap<SubstitutionRuleList, Concept, Role, Boolean, Numerical> m_map;
    SubstitutionRuleLists<Concept, Role, Boolean, Numerical> m_lists;

    static const SubstitutionRuleLists<Concept, Role, Boolean, Numerical> empty_lists;

public:
    SubstitutionRulesContainer() = default;
    SubstitutionRulesContainer(const SubstitutionRulesContainer& other) = delete;
    SubstitutionRulesContainer& operator=(const SubstitutionRulesContainer& other) = delete;
    SubstitutionRulesContainer(SubstitutionRulesContainer&& other) = default;
    SubstitutionRulesContainer& operator=(SubstitutionRulesContainer&& other) = default;

    /**
     * Modifiers
     */

    template<FeatureCategory D>
    void push_back(SubstitutionRule<D> rule)
    {
        boost::hana::at_key(m_map, boost::hana::type<D> {})[rule->get_head()].push_back(rule);
        boost::hana::at_key(m_lists, boost::hana::type<D> {}).push_back(rule);
    }

    /**
     * Accessors
     */

    template<FeatureCategory D>
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

    template<FeatureCategory D>
    const SubstitutionRuleList<D>& get() const
    {
        return boost::hana::at_key(m_lists, boost::hana::type<D> {});
    }

    const SubstitutionRuleLists<Concept, Role, Boolean, Numerical>& get() const { return m_lists; }

    size_t size() const
    {
        return boost::hana::fold(m_lists,
                                 std::size_t { 0 },
                                 [](std::size_t acc, auto pair)
                                 {
                                     return acc + boost::hana::second(pair).size();  // Sum up sizes
                                 });
    }
};

}

#endif
