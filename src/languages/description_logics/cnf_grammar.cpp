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

#include "grammar_cnf_translator.hpp"
#include "parser.hpp"

namespace mimir::dl::cnf_grammar
{

Grammar::Grammar(const grammar::Grammar& grammar)
{
    auto cnf_grammar = translate_to_cnf(grammar);
    m_repositories = std::move(cnf_grammar.m_repositories);
    m_start_symbols = std::move(cnf_grammar.m_start_symbols);
    m_derivation_rules = std::move(cnf_grammar.m_derivation_rules);
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

}
