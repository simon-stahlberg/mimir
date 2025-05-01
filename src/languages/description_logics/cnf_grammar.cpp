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
#include "grammar_bnf/complete.hpp"
#include "grammar_bnf/frances_et_al_aaai2021.hpp"
#include "grammar_cnf_translator.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

NonTerminalToDerivationRuleListMaps initialize_nonterminal_to_derivation_rules(const DerivationRuleLists& derivation_rules)
{
    auto nonterminal_to_derivation_rules = NonTerminalToDerivationRuleListMaps {};

    boost::hana::for_each(derivation_rules,
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              auto& map = boost::hana::at_key(nonterminal_to_derivation_rules, key);

                              for (const auto& rule : second)
                              {
                                  map[rule->get_head()].push_back(rule);
                              }
                          });

    return nonterminal_to_derivation_rules;
}

NonTerminalToSubstitutionRuleListMaps initialize_nonterminal_to_substitution_rules(const SubstitutionRuleLists& substitution_rules)
{
    auto nonterminal_to_substitution_rules = NonTerminalToSubstitutionRuleListMaps {};

    boost::hana::for_each(substitution_rules,
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              auto& map = boost::hana::at_key(nonterminal_to_substitution_rules, key);

                              for (const auto& rule : second)
                              {
                                  map[rule->get_head()].push_back(rule);
                              }
                          });

    return nonterminal_to_substitution_rules;
}

Grammar::Grammar(Repositories repositories,
                 OptionalNonTerminals start_symbols,
                 DerivationRuleLists derivation_rules,
                 SubstitutionRuleLists substitution_rules,
                 Domain domain) :
    m_repositories(std::move(repositories)),
    m_start_symbols(std::move(start_symbols)),
    m_derivation_rules(std::move(derivation_rules)),
    m_substitution_rules(std::move(substitution_rules)),
    m_domain(std::move(domain)),
    m_nonterminal_to_derivation_rules(initialize_nonterminal_to_derivation_rules(m_derivation_rules)),
    m_nonterminal_to_substitution_rules(initialize_nonterminal_to_substitution_rules(m_substitution_rules))
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
    m_nonterminal_to_derivation_rules = initialize_nonterminal_to_derivation_rules(m_derivation_rules);
    m_nonterminal_to_substitution_rules = initialize_nonterminal_to_substitution_rules(m_substitution_rules);
}

Grammar::Grammar(const std::string& bnf_description, Domain domain)
{
    auto grammar = grammar::Grammar(bnf_description, domain);
    auto cnf_grammar = translate_to_cnf(grammar);
    cnf_grammar = simplify(cnf_grammar);
    m_repositories = std::move(cnf_grammar.m_repositories);
    m_start_symbols = std::move(cnf_grammar.m_start_symbols);
    m_derivation_rules = std::move(cnf_grammar.m_derivation_rules);
    m_substitution_rules = std::move(cnf_grammar.m_substitution_rules);
    m_domain = std::move(cnf_grammar.m_domain);
    m_nonterminal_to_derivation_rules = initialize_nonterminal_to_derivation_rules(m_derivation_rules);
    m_nonterminal_to_substitution_rules = initialize_nonterminal_to_substitution_rules(m_substitution_rules);
}

Grammar Grammar::create(GrammarSpecificationEnum type, Domain domain)
{
    switch (type)
    {
        case GrammarSpecificationEnum::COMPLETE:
        {
            return Grammar(create_complete_bnf(domain), domain);
        }
        case GrammarSpecificationEnum::FRANCES_ET_AL_AAAI2021:
        {
            return Grammar(create_frances_et_al_aaai2021_bnf(domain), domain);
        }
        default:
        {
            return Grammar(create_frances_et_al_aaai2021_bnf(domain), domain);
        }
    }
}

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    const auto& start_symbol = boost::hana::at_key(m_start_symbols, boost::hana::type<D> {});

    if (!start_symbol)
    {
        return false;  ///< sentence is not part of language.
    }

    return start_symbol.value()->test_match(constructor, *this);
}

template bool Grammar::test_match(dl::Constructor<ConceptTag> constructor) const;
template bool Grammar::test_match(dl::Constructor<RoleTag> constructor) const;
template bool Grammar::test_match(dl::Constructor<BooleanTag> constructor) const;
template bool Grammar::test_match(dl::Constructor<NumericalTag> constructor) const;

void Grammar::accept(IVisitor& visitor) const { visitor.visit(*this); }

const Repositories& Grammar::get_repositories() const { return m_repositories; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
const OptionalNonTerminal<D>& Grammar::get_start_symbol() const
{
    return boost::hana::at_key(m_start_symbols, boost::hana::type<D> {});
}

template const OptionalNonTerminal<ConceptTag>& Grammar::get_start_symbol() const;
template const OptionalNonTerminal<RoleTag>& Grammar::get_start_symbol() const;
template const OptionalNonTerminal<BooleanTag>& Grammar::get_start_symbol() const;
template const OptionalNonTerminal<NumericalTag>& Grammar::get_start_symbol() const;

const OptionalNonTerminals& Grammar::get_hana_start_symbols() const { return m_start_symbols; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
const DerivationRuleList<D>& Grammar::get_derivation_rules() const
{
    return boost::hana::at_key(m_derivation_rules, boost::hana::type<D> {});
}

template const DerivationRuleList<ConceptTag>& Grammar::get_derivation_rules() const;
template const DerivationRuleList<RoleTag>& Grammar::get_derivation_rules() const;
template const DerivationRuleList<BooleanTag>& Grammar::get_derivation_rules() const;
template const DerivationRuleList<NumericalTag>& Grammar::get_derivation_rules() const;

const DerivationRuleLists& Grammar::get_hana_derivation_rules() const { return m_derivation_rules; }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
const SubstitutionRuleList<D>& Grammar::get_substitution_rules() const
{
    return boost::hana::at_key(m_substitution_rules, boost::hana::type<D> {});
}

template const SubstitutionRuleList<ConceptTag>& Grammar::get_substitution_rules() const;
template const SubstitutionRuleList<RoleTag>& Grammar::get_substitution_rules() const;
template const SubstitutionRuleList<BooleanTag>& Grammar::get_substitution_rules() const;
template const SubstitutionRuleList<NumericalTag>& Grammar::get_substitution_rules() const;

const SubstitutionRuleLists& Grammar::get_hana_substitution_rules() const { return m_substitution_rules; }

const formalism::Domain& Grammar::get_domain() const { return m_domain; }

const NonTerminalToDerivationRuleListMaps& Grammar::get_nonterminal_to_derivation_rules() const { return m_nonterminal_to_derivation_rules; }

const NonTerminalToSubstitutionRuleListMaps& Grammar::get_nonterminal_to_substitution_rules() const { return m_nonterminal_to_substitution_rules; }

}
