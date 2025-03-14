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

class EliminateRulesWithIdenticalBodyNonTerminalVisitor : public CopyVisitor
{
protected:
    const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& m_substitution_map;

public:
    EliminateRulesWithIdenticalBodyNonTerminalVisitor(ConstructorRepositories& repositories,
                                                      StartSymbolsContainer& start_symbols,
                                                      DerivationRulesContainer& derivation_rules,
                                                      SubstitutionRulesContainer& substitution_rules,
                                                      const NonTerminalMap<NonTerminal, Concept, Role, Boolean, Numerical>& substitution_map) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
        m_substitution_map(substitution_map)
    {
    }

    void visit(NonTerminal<Concept> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Role> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Boolean> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor)
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
                                      second.value()->accept(*this);
                                      m_start_symbols.insert(std::any_cast<NonTerminal<FeatureType>>(get_result()));
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

                                      rule->accept(*this);
                                      m_derivation_rules.push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules_container().get(),
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

                                      rule->accept(*this);
                                      m_substitution_rules.push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
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

    auto visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor(repositories, start_symbols, derivation_rules, substitution_rules, substitution_map);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

using HanaSubstitutionNonTerminalOrderings =
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<NonTerminal<Concept>, size_t>>,
                     boost::hana::pair<boost::hana::type<Role>, std::unordered_map<NonTerminal<Role>, size_t>>,
                     boost::hana::pair<boost::hana::type<Boolean>, std::unordered_map<NonTerminal<Boolean>, size_t>>,
                     boost::hana::pair<boost::hana::type<Numerical>, std::unordered_map<NonTerminal<Numerical>, size_t>>>;

class OrderSubstitutionRuleVisitor : public CopyVisitor
{
private:
    const HanaSubstitutionNonTerminalOrderings& m_orderings;

public:
    OrderSubstitutionRuleVisitor(ConstructorRepositories& repositories,
                                 StartSymbolsContainer& start_symbols,
                                 DerivationRulesContainer& derivation_rules,
                                 SubstitutionRulesContainer& substitution_rules,
                                 const HanaSubstitutionNonTerminalOrderings& orderings) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
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
                                      second.value()->accept(*this);
                                      m_start_symbols.insert(std::any_cast<NonTerminal<FeatureType>>(get_result()));
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
                    rule->accept(*this);
                    m_derivation_rules.push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                }
            });

        boost::hana::for_each(
            grammar.get_substitution_rules_container().get(),
            [&](auto&& pair)
            {
                auto key = boost::hana::first(pair);
                auto second = boost::hana::second(pair);
                using FeatureType = typename decltype(+key)::type;

                const auto& ordering = boost::hana::at_key(m_orderings, key);

                std::sort(second.begin(), second.end(), [&](auto&& lhs, auto&& rhs) { return ordering.at(lhs->get_head()) < ordering.at(rhs->get_head()); });

                for (const auto& rule : second)
                {
                    rule->accept(*this);
                    m_substitution_rules.push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                }
            });
    }
};

static Grammar order_substitution_rules(const Grammar& grammar)
{
    auto orderings = HanaSubstitutionNonTerminalOrderings {};

    boost::hana::for_each(grammar.get_substitution_rules_container().get(),
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

                              using Vertex = mimir::graphs::Vertex<NonTerminal<D>>;
                              auto graph = graphs::StaticGraph<Vertex, graphs::EmptyEdge>();

                              auto non_terminal_to_vertex = std::unordered_map<NonTerminal<D>, size_t> {};
                              for (const auto& non_terminal : non_terminals)
                              {
                                  non_terminal_to_vertex.emplace(non_terminal, graph.add_vertex(non_terminal));
                              }

                              for (const auto& rule : second)
                              {
                                  graph.add_directed_edge(non_terminal_to_vertex.at(rule->get_head()), non_terminal_to_vertex.at(rule->get_body()));
                              }

                              auto top_sort = graphs::topological_sort(graphs::DirectionTaggedType(graph, graphs::Forward {}));

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

    auto visitor = OrderSubstitutionRuleVisitor(repositories, start_symbols, derivation_rules, substitution_rules, orderings);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

/**
 * Eliminate epsilon rules
 */

class CollectHeadAndBodyNonTerminalsVisitor : public RecurseVisitor
{
private:
    std::unordered_set<std::string>& m_head_non_terminals;
    std::unordered_set<std::string>& m_body_non_terminals;

public:
    CollectHeadAndBodyNonTerminalsVisitor(std::unordered_set<std::string>& head_non_terminals, std::unordered_set<std::string>& body_non_terminals) :
        m_head_non_terminals(head_non_terminals),
        m_body_non_terminals(body_non_terminals)
    {
    }

    void visit(DerivationRule<Concept> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<Role> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<Boolean> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> constructor)
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        constructor->get_body()->accept(*this);
    }

    void visit(SubstitutionRule<Concept> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<Role> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<Boolean> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
    void visit_impl(SubstitutionRule<D> constructor)
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        m_body_non_terminals.insert(constructor->get_body()->get_name());
    }

    void visit(NonTerminal<Concept> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Role> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Boolean> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<Numerical> constructor) override { visit_impl(constructor); }

    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor)
    {
        m_body_non_terminals.insert(constructor->get_name());
    }
};

std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> collect_head_and_body_nonterminals(const Grammar& grammar)
{
    auto head_nonterminals = std::unordered_set<std::string> {};
    auto body_nonterminals = std::unordered_set<std::string> {};

    auto grammar_visitor = CollectHeadAndBodyNonTerminalsVisitor(head_nonterminals, body_nonterminals);

    grammar.accept(grammar_visitor);

    return { head_nonterminals, body_nonterminals };
}

void verify_grammar_is_well_defined(const Grammar& grammar)
{
    auto [head_nonterminals, body_nonterminals] = collect_head_and_body_nonterminals(grammar);

    /* 1. Verify that all body non-terminals appear in a head. */
    for (const auto& body_nonterminal : body_nonterminals)
    {
        if (!head_nonterminals.contains(body_nonterminal))
        {
            throw std::runtime_error("verify_grammar_is_well_defined(grammar): The body nonterminal " + body_nonterminal + " is never defined in a rule head.");
        }
    }

    /* 2. Verify that grammar has a start symbol. */
    if (!boost::hana::any_of(boost::hana::values(grammar.get_start_symbols_container().get()), [](auto&& value) { return value.has_value(); }))
    {
        throw std::runtime_error("verify_grammar_is_well_defined(grammar): The grammar does not define any start symbol.");
    }

    /* 3. Verify that all start symbols appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && !head_nonterminals.contains(start_non_terminal.value()->get_name()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " is never defined in a rule head.");
                              }
                          });

    /* 4. Verify that all start symbols only appear in a head. */
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto& start_non_terminal = boost::hana::second(pair);

                              if (start_non_terminal.has_value() && body_nonterminals.contains(start_non_terminal.value()->get_name()))
                              {
                                  throw std::runtime_error("verify_grammar_is_well_defined(grammar): The start nonterminal "
                                                           + start_non_terminal.value()->get_name() + " should not appear in a rule body.");
                              }
                          });
}

class IsEpsilonRuleVisitor : public RecurseVisitor
{
protected:
    const std::unordered_set<std::string>& m_epsilon_nonterminals;

    bool m_result;

public:
    IsEpsilonRuleVisitor(const std::unordered_set<std::string>& epsilon_nonterminals) : m_epsilon_nonterminals(epsilon_nonterminals) {}

    void visit(ConceptBot constructor) override { m_result = false; }
    void visit(ConceptTop constructor) override { m_result = false; }
    void visit(ConceptAtomicState<Static> constructor) override { m_result = false; }
    void visit(ConceptAtomicState<Fluent> constructor) override { m_result = false; }
    void visit(ConceptAtomicState<Derived> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<Static> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<Fluent> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<Derived> constructor) override { m_result = false; }
    void visit(ConceptNominal constructor) override { m_result = false; }
    void visit(ConceptIntersection constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_concept()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_concept()->get_name());
    }
    void visit(ConceptUnion constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_concept()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_concept()->get_name());
    }
    void visit(ConceptNegation constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_concept()->get_name()); }
    void visit(ConceptValueRestriction constructor) override
    {
        m_result =
            m_epsilon_nonterminals.contains(constructor->get_role()->get_name()) || m_epsilon_nonterminals.contains(constructor->get_concept()->get_name());
    }
    void visit(ConceptExistentialQuantification constructor) override
    {
        m_result =
            m_epsilon_nonterminals.contains(constructor->get_role()->get_name()) || m_epsilon_nonterminals.contains(constructor->get_concept()->get_name());
    }
    void visit(ConceptRoleValueMapContainment constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_role()->get_name());
    }
    void visit(ConceptRoleValueMapEquality constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_role()->get_name());
    }

    void visit(RoleUniversal constructor) override { m_result = false; }
    void visit(RoleAtomicState<Static> constructor) override { m_result = false; }
    void visit(RoleAtomicState<Fluent> constructor) override { m_result = false; }
    void visit(RoleAtomicState<Derived> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<Static> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<Fluent> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<Derived> constructor) override { m_result = false; }
    void visit(RoleIntersection constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_role()->get_name());
    }
    void visit(RoleUnion constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_role()->get_name());
    }
    void visit(RoleComplement constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_role()->get_name()); }
    void visit(RoleInverse constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_role()->get_name()); }
    void visit(RoleComposition constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_role()->get_name());
    }
    void visit(RoleTransitiveClosure constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_role()->get_name()); }
    void visit(RoleReflexiveTransitiveClosure constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_role()->get_name()); }
    void visit(RoleRestriction constructor) override
    {
        m_result =
            m_epsilon_nonterminals.contains(constructor->get_role()->get_name()) || m_epsilon_nonterminals.contains(constructor->get_concept()->get_name());
    }
    void visit(RoleIdentity constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_concept()->get_name()); }

    void visit(BooleanAtomicState<Static> constructor) override { m_result = false; }
    void visit(BooleanAtomicState<Fluent> constructor) override { m_result = false; }
    void visit(BooleanAtomicState<Derived> constructor) override { m_result = false; }
    void visit(BooleanNonempty<Concept> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(BooleanNonempty<Role> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }

    void visit(NumericalCount<Concept> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(NumericalCount<Role> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(NumericalDistance constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_concept()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_concept()->get_name());
    }

    bool get_result() const { return m_result; }
};

template<FeatureCategory D>
bool is_epsilon_rule(DerivationRule<D> rule, const std::unordered_set<std::string>& epsilon_nonterminals)
{
    auto visitor = IsEpsilonRuleVisitor(epsilon_nonterminals);
    rule->get_body()->accept(visitor);
    return visitor.get_result();
}

template<FeatureCategory D>
bool is_epsilon_rule(SubstitutionRule<D> rule, const std::unordered_set<std::string>& epsilon_nonterminals)
{
    return epsilon_nonterminals.contains(rule->get_body()->get_name());
}

class EliminateEpsilonRuleVisitor : public CopyVisitor
{
private:
    const std::unordered_set<std::string>& m_epsilon_nonterminals;

public:
    EliminateEpsilonRuleVisitor(ConstructorRepositories& repositories,
                                StartSymbolsContainer& start_symbols,
                                DerivationRulesContainer& derivation_rules,
                                SubstitutionRulesContainer& substitution_rules,
                                const std::unordered_set<std::string>& epsilon_nonterminals) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
        m_epsilon_nonterminals(epsilon_nonterminals)
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
                                      second.value()->accept(*this);
                                      m_start_symbols.insert(std::any_cast<NonTerminal<FeatureType>>(get_result()));
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  auto second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (is_epsilon_rule(rule, m_epsilon_nonterminals))
                                      {
                                          continue;
                                      }

                                      rule->accept(*this);
                                      m_derivation_rules.push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  auto second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (is_epsilon_rule(rule, m_epsilon_nonterminals))
                                      {
                                          continue;
                                      }

                                      rule->accept(*this);
                                      m_substitution_rules.push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                                  }
                              });
    }
};

Grammar eliminate_epsilon_rules(const Grammar& grammar)
{
    auto [head_nonterminals, body_nonterminals] = collect_head_and_body_nonterminals(grammar);

    for (const auto& head_nonterminal : head_nonterminals)
    {
        body_nonterminals.erase(head_nonterminal);
    }

    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();
    auto visitor = EliminateEpsilonRuleVisitor(repositories, start_symbols, derivation_rules, substitution_rules, body_nonterminals);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

Grammar simplify(const Grammar& grammar)
{
    /* Step 1 Identify rules with same body */

    auto simplified_grammar = eliminate_rules_with_identical_body(grammar);

    /* Step 2 eliminate epsilon rules */

    size_t num_rules_before;
    size_t num_rules_after;
    do
    {
        num_rules_before = simplified_grammar.get_derivation_rules_container().size() + grammar.get_substitution_rules_container().size();

        simplified_grammar = eliminate_epsilon_rules(simplified_grammar);

        num_rules_after = simplified_grammar.get_derivation_rules_container().size() + grammar.get_substitution_rules_container().size();
    } while (num_rules_before != num_rules_after);

    /* Step 3 order substitution rules in order of evaluation through topological sorting */

    simplified_grammar = order_substitution_rules(simplified_grammar);

    /* Step 4 remove rules of unreachable non-terminals */

    return simplified_grammar;
}
}
