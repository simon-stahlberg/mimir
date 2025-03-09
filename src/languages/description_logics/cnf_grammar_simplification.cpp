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

#include "cnf_grammar_simplification.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/graphs/static_graph_boost_adapter.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"

namespace mimir::dl::cnf_grammar
{

template<FeatureCategory D>
class EliminateRulesWithIdenticalBodyNonTerminalVisitor : public CopyNonTerminalVisitor<D>
{
protected:
    const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& m_substitution_map;

public:
    EliminateRulesWithIdenticalBodyNonTerminalVisitor(ConstructorRepositories& repositories,
                                                      const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& substitution_map) :
        CopyNonTerminalVisitor<D>(repositories),
        m_substitution_map(substitution_map)
    {
    }

    void visit(NonTerminal<D> constructor) override
    {
        const auto& substitution_map = boost::hana::at_key(m_substitution_map, boost::hana::type<D> {});

        const auto it = substitution_map.find(constructor);

        if (it != substitution_map.end())
        {
            this->m_result = this->m_repositories.template get_or_create_nonterminal<D>(it->second->get_name());
        }
        else
        {
            this->m_result = this->m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
        }
    }
};

template<FeatureCategory... Ds>
using HanaEliminateRulesWithIdenticalBodyNonTerminalVisitors =
    boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<EliminateRulesWithIdenticalBodyNonTerminalVisitor<Ds>>>...>;

class EliminateRulesWithIdenticalBodyGrammarVisitor : public CopyGrammarVisitor
{
private:
    const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& m_substitution_map;

public:
    EliminateRulesWithIdenticalBodyGrammarVisitor(ConstructorRepositories& repositories,
                                                  StartSymbolsContainer& start_symbols,
                                                  DerivationRulesContainer& derivation_rules,
                                                  SubstitutionRulesContainer& substitution_rules,
                                                  HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                                                  HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors,
                                                  HanaCopySubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> substitution_rule_visitors,
                                                  const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& substitution_map) :
        CopyGrammarVisitor(repositories,
                           start_symbols,
                           derivation_rules,
                           substitution_rules,
                           start_symbol_visitors,
                           derivation_rule_visitors,
                           substitution_rule_visitors),
        m_substitution_map(substitution_map)
    {
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_start_symbols_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      auto& visitor = get_visitor<FeatureType>(m_start_symbol_visitor);
                                      second.value()->accept(visitor);
                                      m_start_symbols.insert(visitor.get_result());
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (boost::hana::at_key(m_substitution_map, boost::hana::type<FeatureType> {}).contains(rule->get_head()))
                                      {
                                          continue;  ///< non-terminal will be substituted, which renders the rule useless.
                                      }

                                      auto& visitor = get_visitor<FeatureType>(m_derivation_rule_visitor);
                                      rule->accept(visitor);
                                      const auto copied_rule = visitor.get_result();
                                      m_derivation_rules.push_back(copied_rule);
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (boost::hana::at_key(m_substitution_map, boost::hana::type<FeatureType> {}).contains(rule->get_head()))
                                      {
                                          continue;  ///< non-terminal will be substituted, which renders the rule useless.
                                      }

                                      auto& visitor = get_visitor<FeatureType>(m_substitution_rule_visitor);
                                      rule->accept(visitor);
                                      const auto copied_rule = visitor.get_result();
                                      m_substitution_rules.push_back(copied_rule);
                                  }
                              });
    }
};

static Grammar eliminate_rules_with_identical_body(const Grammar& grammar)
{
    auto inverse_derivation_rules = ConstructorMap<NonTerminalList, Concept, Role, Boolean, Numerical>();

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& rule : second)
                              {
                                  boost::hana::at_key(inverse_derivation_rules, key)[rule->get_body()].push_back(rule->get_head());
                              }
                          });

    auto substitution_map = NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>();

    boost::hana::for_each(inverse_derivation_rules,
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& body_to_heads = boost::hana::second(pair);

                              for (const auto& body_and_heads : body_to_heads)
                              {
                                  const auto& heads = body_and_heads.second;

                                  const auto& representative_head = heads.front();

                                  for (auto it = std::next(heads.begin()); it != heads.end(); ++it)
                                  {
                                      boost::hana::at_key(substitution_map, key).emplace(*it, representative_head);
                                  }
                              }
                          });

    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();

    auto concept_eliminate_nonterminal_visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor<Concept>(repositories, substitution_map);
    auto role_eliminate_nonterminal_visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor<Role>(repositories, substitution_map);
    auto boolean_eliminate_nonterminal_visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor<Boolean>(repositories, substitution_map);
    auto numerical_eliminate_nonterminal_visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor<Numerical>(repositories, substitution_map);

    auto substitute_nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<CopyNonTerminalVisitor<Concept>&>(concept_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<CopyNonTerminalVisitor<Role>&>(role_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(static_cast<CopyNonTerminalVisitor<Boolean>&>(boolean_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Numerical> {},
                               std::ref(static_cast<CopyNonTerminalVisitor<Numerical>&>(numerical_eliminate_nonterminal_visitor))));

    auto concept_and_role_eliminate_nonterminal_visitor = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<CopyNonTerminalVisitor<Concept>&>(concept_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<CopyNonTerminalVisitor<Role>&>(role_eliminate_nonterminal_visitor))));

    auto concept_constructor_visitor = CopyConstructorVisitor<Concept>(repositories, concept_and_role_eliminate_nonterminal_visitor);
    auto role_constructor_visitor = CopyConstructorVisitor<Role>(repositories, concept_and_role_eliminate_nonterminal_visitor);
    auto boolean_constructor_visitor = CopyConstructorVisitor<Boolean>(repositories, concept_and_role_eliminate_nonterminal_visitor);
    auto numerical_constructor_visitor = CopyConstructorVisitor<Numerical>(repositories, concept_and_role_eliminate_nonterminal_visitor);

    auto concept_derivation_rule_visitor = CopyDerivationRuleVisitor<Concept>(repositories, concept_eliminate_nonterminal_visitor, concept_constructor_visitor);
    auto role_derivation_rule_visitor = CopyDerivationRuleVisitor<Role>(repositories, role_eliminate_nonterminal_visitor, role_constructor_visitor);
    auto boolean_derivation_rule_visitor = CopyDerivationRuleVisitor<Boolean>(repositories, boolean_eliminate_nonterminal_visitor, boolean_constructor_visitor);
    auto numerical_derivation_rule_visitor =
        CopyDerivationRuleVisitor<Numerical>(repositories, numerical_eliminate_nonterminal_visitor, numerical_constructor_visitor);

    auto derivation_rule_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(concept_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(role_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(boolean_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Numerical> {}, std::ref(numerical_derivation_rule_visitor)));

    auto concept_substitution_rule_visitor = CopySubstitutionRuleVisitor<Concept>(repositories, concept_eliminate_nonterminal_visitor);
    auto role_substitution_rule_visitor = CopySubstitutionRuleVisitor<Role>(repositories, role_eliminate_nonterminal_visitor);
    auto boolean_substitution_rule_visitor = CopySubstitutionRuleVisitor<Boolean>(repositories, boolean_eliminate_nonterminal_visitor);
    auto numerical_substitution_rule_visitor = CopySubstitutionRuleVisitor<Numerical>(repositories, numerical_eliminate_nonterminal_visitor);

    auto substitution_rule_visitors =
        boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(concept_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(role_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(boolean_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Numerical> {}, std::ref(numerical_substitution_rule_visitor)));

    auto grammar_visitor = EliminateRulesWithIdenticalBodyGrammarVisitor(repositories,
                                                                         start_symbols,
                                                                         derivation_rules,
                                                                         substitution_rules,
                                                                         substitute_nonterminal_visitors,
                                                                         derivation_rule_visitors,
                                                                         substitution_rule_visitors,
                                                                         substitution_map);

    grammar_visitor.visit(grammar);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

using HanaSubstitutionNonTerminalOrderings =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, size_t>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, size_t>>,
                     boost::hana::pair<boost::hana::type<Boolean>, std::unordered_map<NonTerminal<Boolean>, size_t>>,
                     boost::hana::pair<boost::hana::type<Numerical>, std::unordered_map<NonTerminal<Numerical>, size_t>>>;

class OrderSubstitutionRuleGrammarVisitor : public CopyGrammarVisitor
{
private:
    const HanaSubstitutionNonTerminalOrderings& m_orderings;

public:
    OrderSubstitutionRuleGrammarVisitor(ConstructorRepositories& repositories,
                                        StartSymbolsContainer& start_symbols,
                                        DerivationRulesContainer& derivation_rules,
                                        SubstitutionRulesContainer& substitution_rules,
                                        HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                                        HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors,
                                        HanaCopySubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> substitution_rule_visitors,
                                        const HanaSubstitutionNonTerminalOrderings& orderings) :
        CopyGrammarVisitor(repositories,
                           start_symbols,
                           derivation_rules,
                           substitution_rules,
                           start_symbol_visitors,
                           derivation_rule_visitors,
                           substitution_rule_visitors),
        m_orderings(orderings)
    {
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_start_symbols_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      auto& visitor = get_visitor<FeatureType>(m_start_symbol_visitor);
                                      second.value()->accept(visitor);
                                      m_start_symbols.insert(visitor.get_result());
                                  }
                              });

        boost::hana::for_each(
            grammar.get_derivation_rules_container().get(),
            [&](auto&& pair)
            {
                auto key = boost::hana::first(pair);
                auto second = boost::hana::second(pair);
                using FeatureType = typename decltype(+key)::type;

                std::sort(second.begin(), second.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_head()->get_name() < rhs->get_head()->get_name(); });

                for (const auto& rule : second)
                {
                    auto& visitor = get_visitor<FeatureType>(m_derivation_rule_visitor);
                    rule->accept(visitor);
                    const auto copied_rule = visitor.get_result();
                    m_derivation_rules.push_back(copied_rule);
                }
            });

        boost::hana::for_each(
            grammar.get_substitution_rules().get(),
            [&](auto&& pair)
            {
                auto key = boost::hana::first(pair);
                auto second = boost::hana::second(pair);
                using FeatureType = typename decltype(+key)::type;

                const auto& ordering = boost::hana::at_key(m_orderings, key);

                std::sort(second.begin(), second.end(), [&](auto&& lhs, auto&& rhs) { return ordering.at(lhs->get_head()) < ordering.at(rhs->get_head()); });

                for (const auto& rule : second)
                {
                    auto& visitor = get_visitor<FeatureType>(m_substitution_rule_visitor);
                    rule->accept(visitor);
                    const auto copied_rule = visitor.get_result();
                    m_substitution_rules.push_back(copied_rule);
                }
            });
    }
};

static Grammar order_substitution_rules(const Grammar& grammar)
{
    auto orderings = HanaSubstitutionNonTerminalOrderings {};

    boost::hana::for_each(grammar.get_substitution_rules().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using D = typename decltype(+key)::type;

                              auto non_terminals = NonTerminalSet<D> {};
                              for (const auto& rule : second)
                              {
                                  non_terminals.insert(rule->get_head());
                                  non_terminals.insert(rule->get_body());
                              }

                              using Vertex = mimir::Vertex<NonTerminal<D>>;
                              auto graph = StaticGraph<Vertex, EmptyEdge>();

                              auto non_terminal_to_vertex = std::unordered_map<NonTerminal<D>, size_t> {};
                              for (const auto& non_terminal : non_terminals)
                              {
                                  non_terminal_to_vertex.emplace(non_terminal, graph.add_vertex(non_terminal));
                              }

                              for (const auto& rule : second)
                              {
                                  graph.add_directed_edge(non_terminal_to_vertex.at(rule->get_head()), non_terminal_to_vertex.at(rule->get_body()));
                              }

                              auto top_sort = topological_sort(TraversalDirectionTaggedType(graph, ForwardTraversal {}));

                              for (size_t i = 0; i < top_sort.size(); ++i)
                              {
                                  const auto& vertex = graph.get_vertex(top_sort.at(i));
                                  const auto nonterminal = vertex.template get_property<0>();
                                  non_terminal_to_vertex.at(nonterminal) = i;
                              }

                              boost::hana::at_key(orderings, key) = std::move(non_terminal_to_vertex);
                          });

    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();

    auto concept_eliminate_nonterminal_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_eliminate_nonterminal_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto boolean_eliminate_nonterminal_visitor = CopyNonTerminalVisitor<Boolean>(repositories);
    auto numerical_eliminate_nonterminal_visitor = CopyNonTerminalVisitor<Numerical>(repositories);

    auto nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<CopyNonTerminalVisitor<Concept>&>(concept_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<CopyNonTerminalVisitor<Role>&>(role_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(static_cast<CopyNonTerminalVisitor<Boolean>&>(boolean_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Numerical> {},
                               std::ref(static_cast<CopyNonTerminalVisitor<Numerical>&>(numerical_eliminate_nonterminal_visitor))));

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(static_cast<CopyNonTerminalVisitor<Concept>&>(concept_eliminate_nonterminal_visitor))),
        boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(static_cast<CopyNonTerminalVisitor<Role>&>(role_eliminate_nonterminal_visitor))));

    auto concept_constructor_visitor = CopyConstructorVisitor<Concept>(repositories, concept_and_role_nonterminal_visitors);
    auto role_constructor_visitor = CopyConstructorVisitor<Role>(repositories, concept_and_role_nonterminal_visitors);
    auto boolean_constructor_visitor = CopyConstructorVisitor<Boolean>(repositories, concept_and_role_nonterminal_visitors);
    auto numerical_constructor_visitor = CopyConstructorVisitor<Numerical>(repositories, concept_and_role_nonterminal_visitors);

    auto concept_derivation_rule_visitor = CopyDerivationRuleVisitor<Concept>(repositories, concept_eliminate_nonterminal_visitor, concept_constructor_visitor);
    auto role_derivation_rule_visitor = CopyDerivationRuleVisitor<Role>(repositories, role_eliminate_nonterminal_visitor, role_constructor_visitor);
    auto boolean_derivation_rule_visitor = CopyDerivationRuleVisitor<Boolean>(repositories, boolean_eliminate_nonterminal_visitor, boolean_constructor_visitor);
    auto numerical_derivation_rule_visitor =
        CopyDerivationRuleVisitor<Numerical>(repositories, numerical_eliminate_nonterminal_visitor, numerical_constructor_visitor);

    auto derivation_rule_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(concept_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(role_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(boolean_derivation_rule_visitor)),
                                                          boost::hana::make_pair(boost::hana::type<Numerical> {}, std::ref(numerical_derivation_rule_visitor)));

    auto concept_substitution_rule_visitor = CopySubstitutionRuleVisitor<Concept>(repositories, concept_eliminate_nonterminal_visitor);
    auto role_substitution_rule_visitor = CopySubstitutionRuleVisitor<Role>(repositories, role_eliminate_nonterminal_visitor);
    auto boolean_substitution_rule_visitor = CopySubstitutionRuleVisitor<Boolean>(repositories, boolean_eliminate_nonterminal_visitor);
    auto numerical_substitution_rule_visitor = CopySubstitutionRuleVisitor<Numerical>(repositories, numerical_eliminate_nonterminal_visitor);

    auto substitution_rule_visitors =
        boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, std::ref(concept_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Role> {}, std::ref(role_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Boolean> {}, std::ref(boolean_substitution_rule_visitor)),
                              boost::hana::make_pair(boost::hana::type<Numerical> {}, std::ref(numerical_substitution_rule_visitor)));

    auto grammar_visitor = OrderSubstitutionRuleGrammarVisitor(repositories,
                                                               start_symbols,
                                                               derivation_rules,
                                                               substitution_rules,
                                                               nonterminal_visitors,
                                                               derivation_rule_visitors,
                                                               substitution_rule_visitors,
                                                               orderings);

    grammar_visitor.visit(grammar);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

Grammar simplify(const Grammar& grammar)
{
    /* Step 1 Identify rules with same body */

    auto simplified_grammar = eliminate_rules_with_identical_body(grammar);

    /* Step 2 resubstitute substitution rules using fixed point */

    // If right handside of substitution rule has a single rule

    /* Step 3 order substitution rules in order of evaluation through topological sorting */

    simplified_grammar = order_substitution_rules(simplified_grammar);

    /* Step 4 remove rules of unreachable non-terminals */

    return simplified_grammar;
}
}
