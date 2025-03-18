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

#include "grammar_parser.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

Grammar::Grammar(Repositories repositories, StartSymbolsContainer start_symbols, DerivationRulesContainer derivation_rules, Domain domain) :
    m_repositories(std::move(repositories)),
    m_start_symbols(std::move(start_symbols)),
    m_derivation_rules(std::move(derivation_rules)),
    m_domain(std::move(domain))
{
}

Grammar::Grammar(std::string bnf_description, Domain domain)
{
    auto grammar = parse_grammar(bnf_description, std::move(domain));
    m_repositories = std::move(grammar.m_repositories);
    m_start_symbols = std::move(grammar.m_start_symbols);
    m_derivation_rules = std::move(grammar.m_derivation_rules);
    m_domain = std::move(grammar.m_domain);
}

Grammar::Grammar(GrammarSpecificationEnum type, Domain domain)
{
    // TODO
}

template<FeatureCategory D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    const auto& start_symbol = m_start_symbols.template get<D>();

    if (!start_symbol)
    {
        return false;  ///< sentence is not part of language.
    }

    return start_symbol.value()->test_match(constructor, *this);
}

template bool Grammar::test_match(dl::Constructor<Concept> constructor) const;
template bool Grammar::test_match(dl::Constructor<Role> constructor) const;

void Grammar::accept(IVisitor& visitor) const { visitor.visit(*this); }

const StartSymbolsContainer& Grammar::get_start_symbols_container() const { return m_start_symbols; }

const DerivationRulesContainer& Grammar::get_derivation_rules_container() const { return m_derivation_rules; }

const Domain& Grammar::get_domain() const { return m_domain; }

}
