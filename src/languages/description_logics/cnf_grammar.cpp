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
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
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
}

static std::string create_frances_et_al_aaai2021_bnf(Domain domain)
{
    auto ss = std::stringstream {};
    ss << "[start_symbols]" << "\n"
       << "    concept = <concept_start>" << "\n"
       << "    role = <role_start>" << "\n"
       << "[grammar_rules]" << "\n"
       << "    <concept_start> ::= <concept>" << "\n"
       << "    <role_start> ::= <role>" << "\n";

    auto head_names = HanaContainer<std::vector<std::string>, Concept, Role> {};

    boost::hana::for_each(domain->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& predicates = boost::hana::second(pair);

                              for (const auto& predicate : predicates)
                              {
                                  if (predicate->get_arity() == 1)
                                  {
                                      {
                                          auto head_name = "<concept_atomic_state_" + predicate->get_name() + ">";
                                          boost::hana::at_key(head_names, boost::hana::type<Concept> {}).push_back(head_name);

                                          auto body_name = std::string(dl::keywords::concept_atomic_state) + " \"" + predicate->get_name() + "\"";

                                          ss << "    " << head_name << " ::= " << body_name << "\n";
                                      }

                                      for (const auto& polarity : { "true", "false" })
                                      {
                                          auto head_name = std::string("<concept_atomic_goal_") + polarity + "_" + predicate->get_name() + ">";
                                          boost::hana::at_key(head_names, boost::hana::type<Concept> {}).push_back(head_name);

                                          auto body_name = std::string(dl::keywords::concept_atomic_goal) + " \"" + predicate->get_name() + "\" " + polarity;

                                          ss << "    " << head_name << " ::= " << body_name << "\n";
                                      }
                                  }
                                  else if (predicate->get_arity() == 2)
                                  {
                                      {
                                          auto head_name = "<role_atomic_state_" + predicate->get_name() + ">";
                                          boost::hana::at_key(head_names, boost::hana::type<Role> {}).push_back(head_name);

                                          auto body_name = std::string(dl::keywords::role_atomic_state) + " \"" + predicate->get_name() + "\"";

                                          ss << "    " << head_name << " ::= " << body_name << "\n";
                                      }

                                      for (const auto& polarity : { "true", "false" })
                                      {
                                          auto head_name = std::string("<role_atomic_goal_") + polarity + "_" + predicate->get_name() + ">";
                                          boost::hana::at_key(head_names, boost::hana::type<Role> {}).push_back(head_name);

                                          auto body_name = std::string(dl::keywords::role_atomic_goal) + " \"" + predicate->get_name() + "\" " + polarity;

                                          ss << "    " << head_name << " ::= " << body_name << "\n";
                                      }
                                  }
                              }
                          });

    boost::hana::for_each(head_names,
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using KeyType = typename decltype(+key)::type;

                              if constexpr (std::same_as<KeyType, Concept>)
                              {
                                  ss << "    <concept> ::= ";
                              }
                              else if constexpr (std::same_as<KeyType, Role>)
                              {
                                  ss << "    <role> ::= ";
                              }

                              bool first = true;
                              for (const auto& head_name : second)
                              {
                                  if (!first)
                                      ss << " | ";

                                  ss << head_name;

                                  first = false;
                              }
                              ss << "\n";
                          });

    std::cout << ss.str() << std::endl;

    return ss.str();
}

Grammar Grammar::create(GrammarSpecificationEnum type, Domain domain)
{
    switch (type)
    {
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
