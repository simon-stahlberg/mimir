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

#include "mimir/languages/description_logics/cnf_grammar.hpp"

#include "cnf_grammar_simplification.hpp"
#include "grammar_cnf_translator.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"
#include "parser.hpp"

namespace mimir::dl::cnf_grammar
{

Grammar::Grammar(ConstructorRepositories repositories,
                 StartSymbolsContainer start_symbols,
                 DerivationRulesContainer derivation_rules,
                 SubstitutionRulesContainer substitution_rules,
                 Domain domain) :
    m_repositories(std::move(repositories)),
    m_start_symbols(std::move(start_symbols)),
    m_derivation_rules(std::move(derivation_rules)),
    m_substitution_rules(std::move(substitution_rules)),
    m_domain(std::move(domain))
{
}

Grammar::Grammar(const grammar::Grammar& grammar)
{
    auto cnf_grammar = translate_to_cnf(grammar);
    cnf_grammar = simplify(cnf_grammar);
    m_repositories = std::move(cnf_grammar.m_repositories);
    m_start_symbols = std::move(cnf_grammar.m_start_symbols);
    m_derivation_rules = std::move(cnf_grammar.m_derivation_rules);
    m_substitution_rules = std::move(cnf_grammar.m_substitution_rules);
    m_domain = std::move(cnf_grammar.m_domain);
}

template<ConceptOrRole D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    const auto& start_symbol = m_start_symbols.get<D>();

    if (!start_symbol)
    {
        return false;  ///< sentence is not part of language.
    }

    return start_symbol.value()->test_match(constructor, *this);
}

template bool Grammar::test_match(dl::Constructor<Concept> constructor) const;
template bool Grammar::test_match(dl::Constructor<Role> constructor) const;

void Grammar::accept(GrammarVisitor& visitor) const { visitor.visit(*this); }

}
