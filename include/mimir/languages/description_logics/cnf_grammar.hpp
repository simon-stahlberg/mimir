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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructor_repositories.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <loki/loki.hpp>

namespace mimir::dl::cnf_grammar
{

using StartSymbols = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::optional<NonTerminal<Concept>>>,
                                      boost::hana::pair<boost::hana::type<Role>, std::optional<NonTerminal<Role>>>>;

using GrammarRules = boost::hana::map<
    boost::hana::pair<
        boost::hana::type<Concept>,
        boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, std::unordered_map<NonTerminal<Concept>, ConstructorList<Concept, Primitive>>>,
                         boost::hana::pair<boost::hana::type<Composite>, std::unordered_map<NonTerminal<Concept>, ConstructorList<Concept, Composite>>>>>,
    boost::hana::pair<
        boost::hana::type<Role>,
        boost::hana::map<boost::hana::pair<boost::hana::type<Primitive>, std::unordered_map<NonTerminal<Role>, ConstructorList<Role, Primitive>>>,
                         boost::hana::pair<boost::hana::type<Composite>, std::unordered_map<NonTerminal<Role>, ConstructorList<Role, Composite>>>>>>;

enum class GrammarSpecificationEnum
{
    FRANCES_ET_AL_AAAI2021 = 0,
};

class Grammar
{
private:
    /* Memory */
    GrammarConstructorRepositories m_repositories;

    /* The rules of the grammar. */
    StartSymbols m_start_symbols;
    GrammarRules m_rules;

public:
    /// @brief Translate a grammar to CNF.
    explicit Grammar(const grammar::Grammar& grammar);

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

    template<ConceptOrRole D, PrimitiveOrComposite C>
    const ConstructorList<D, C>& get_rules(NonTerminal<D> non_terminal) const
    {
        return boost::hana::at_key(boost::hana::at_key(m_rules, boost::hana::type<D> {}), boost::hana::type<C> {}).at(non_terminal);
    }

    const GrammarRules& get_rules() const { return m_rules; }
};
}

#endif
