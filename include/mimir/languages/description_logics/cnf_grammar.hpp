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

namespace mimir::languages::dl::cnf_grammar
{

enum class GrammarSpecificationEnum
{
    FRANCES_ET_AL_AAAI2021 = 0,
};

class Grammar
{
private:
    /* Memory */
    Repositories m_repositories;

    /* The rules of the grammar. */
    OptionalNonTerminals m_start_symbols;
    DerivationRuleLists m_derivation_rules;
    SubstitutionRuleLists m_substitution_rules;

    formalism::Domain m_domain;

    /* Initialized in the constructor for ease of lookup. */
    NonTerminalToDerivationRuleListMaps m_nonterminal_to_derivation_rules;
    NonTerminalToSubstitutionRuleListMaps m_nonterminal_to_substitution_rules;

public:
    Grammar(Repositories repositories,
            OptionalNonTerminals start_symbols,
            DerivationRuleLists derivation_rules,
            SubstitutionRuleLists substitution_rules,
            formalism::Domain domain);

    explicit Grammar(const grammar::Grammar& grammar);

    Grammar(const std::string& bnf_description, formalism::Domain domain);

    static Grammar create(GrammarSpecificationEnum type, formalism::Domain domain);

    /// @brief Tests whether a dl constructor satisfies the grammar specification.
    /// @param constructor is the dl constructor to test.
    /// @return true iff the dl constructor satisfies the grammar specification, and false otherwise.
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    bool test_match(dl::Constructor<D> constructor) const;

    void accept(IVisitor& visitor) const;

    /**
     * Getters
     */

    const Repositories& get_repositories() const { return m_repositories; }
    const OptionalNonTerminals& get_start_symbols_container() const { return m_start_symbols; }
    const DerivationRuleLists& get_derivation_rules() const { return m_derivation_rules; }
    const SubstitutionRuleLists& get_substitution_rules() const { return m_substitution_rules; }
    const formalism::Domain& get_domain() const { return m_domain; }
    const NonTerminalToDerivationRuleListMaps& get_nonterminal_to_derivation_rules() const;
    const NonTerminalToSubstitutionRuleListMaps& get_nonterminal_to_substitution_rules() const;
};
}

#endif
