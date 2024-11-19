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

class Grammar
{
private:
    /* Memory */
    ConstructorTagToRepository m_grammar_constructor_repos;

    /* The rules of the grammar. */
    ConstructorTagToDerivationRuleList m_primitive_production_rules;
    ConstructorTagToDerivationRuleList m_composite_production_rules;
    ConstructorTagToDerivationRuleList m_alternative_rules;

public:
    /// @brief Create a grammar from a BNF description.
    Grammar(std::string bnf_description, Domain domain);

    /// @brief Tests whether a dl constructor satisfies the grammar specification.
    /// @param constructor is the dl constructor to test.
    /// @return true iff the dl constructor satisfies the grammar specification, and false otherwise.
    template<ConstructorTag D>
    bool test_match(dl::Constructor<D> constructor) const;

    /**
     * Getters
     */

    template<ConstructorTag D>
    const DerivationRuleList<D>& get_primitive_production_rules() const;

    template<ConstructorTag D>
    const DerivationRuleList<D>& get_composite_production_rules() const;

    template<ConstructorTag D>
    const DerivationRuleList<D>& get_alternative_rules() const;
};
}

#endif
