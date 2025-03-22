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

namespace mimir::languages::dl::grammar
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
    DerivationRuleSets m_derivation_rules;

    formalism::Domain m_domain;

    /* Initialized in the constructor for ease of lookup. */
    NonTerminalToDerivationRuleSets m_nonterminal_to_derivation_rules;

public:
    Grammar(Repositories repositories, OptionalNonTerminals start_symbols, DerivationRuleSets derivation_rules, formalism::Domain domain);

    /// @brief Create a grammar from a BNF description for a given domain.
    /// The domain is used for error checking only to ensure that predicates or constants are available.
    Grammar(std::string bnf_description, formalism::Domain domain);

    /// @brief Create a predefined grammar for a given domain.
    Grammar(GrammarSpecificationEnum type, formalism::Domain domain);

    /// @brief Tests whether a dl constructor satisfies the grammar specification.
    /// @param constructor is the dl constructor to test.
    /// @return true iff the dl constructor satisfies the grammar specification, and false otherwise.
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    bool test_match(dl::Constructor<D> constructor) const;

    /**
     * Visitor
     */

    void accept(IVisitor& visitor) const;

    /**
     * Getters
     */

    const OptionalNonTerminals& get_start_symbols() const;
    const DerivationRuleSets& get_derivation_rules() const;
    const formalism::Domain& get_domain() const;
    const NonTerminalToDerivationRuleSets& get_nonterminal_to_derivation_rules() const;
};
}

#endif
