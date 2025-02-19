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

#include "mimir/languages/description_logics/grammar.hpp"

#include "mimir/languages/description_logics/grammar_verification.hpp"
#include "mimir/languages/description_logics/parser/parser.hpp"
#include "mimir/languages/description_logics/parser/parser_wrapper.hpp"
#include "parser.hpp"

namespace mimir::dl::grammar
{

Grammar::Grammar() : m_repositories(), m_start_symbols(), m_rules() {}

Grammar::Grammar(GrammarConstructorRepositories repositories, StartSymbols start_symbols, GrammarRules rules) :
    m_repositories(std::move(repositories)),
    m_start_symbols(std::move(start_symbols)),
    m_rules(std::move(rules))
{
    verify_grammar_is_well_defined(*this);
}

Grammar Grammar::parse(std::string bnf_description, Domain domain) { return ::mimir::dl::grammar::parse(bnf_description, domain); }

Grammar Grammar::create(GrammarSpecificationEnum type, Domain domain)
{
    // TODO
}

template<ConceptOrRole D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    const auto& start_symbol = get_start_symbol<D>();

    if (!start_symbol)
    {
        return false;  ///< sentence is not part of language.
    }

    const auto& rules = get_rules<D>(start_symbol.value());

    return std::any_of(rules.begin(), rules.end(), [&, constructor](auto&& rule) { return rule->test_match(constructor, *this); });
}

template bool Grammar::test_match(dl::Constructor<Concept> constructor) const;
template bool Grammar::test_match(dl::Constructor<Role> constructor) const;

}
