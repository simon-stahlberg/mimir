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
#include "mimir/graphs/bgl/graph_algorithms.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

class EliminateRulesWithIdenticalBodyNonTerminalVisitor : public CopyVisitor
{
protected:
    const NonTerminalToNonTerminalMaps& m_substitution_map;

public:
    EliminateRulesWithIdenticalBodyNonTerminalVisitor(Repositories& repositories,
                                                      OptionalNonTerminals& start_symbols,
                                                      DerivationRuleLists& derivation_rules,
                                                      SubstitutionRuleLists& substitution_rules,
                                                      const NonTerminalToNonTerminalMaps& substitution_map) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
        m_substitution_map(substitution_map)
    {
    }

    void visit(NonTerminal<ConceptTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<RoleTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<BooleanTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
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
        boost::hana::for_each(grammar.get_start_symbols_container(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      second.value()->accept(*this);
                                      boost::hana::at_key(m_start_symbols, key) = std::any_cast<NonTerminal<FeatureType>>(get_result());
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules(),
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
                                      boost::hana::at_key(m_derivation_rules, key).push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules(),
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
                                      boost::hana::at_key(m_substitution_rules, key).push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                                  }
                              });
    }
};

static Grammar eliminate_rules_with_identical_body(const Grammar& grammar)
{
    auto inverse_derivation_rules = ConstructorToNonTerminalListMaps();

    boost::hana::for_each(grammar.get_derivation_rules(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& rule : second)
                              {
                                  boost::hana::at_key(inverse_derivation_rules, key)[rule->get_body()].push_back(rule->get_head());
                              }
                          });

    auto substitution_map = NonTerminalToNonTerminalMaps();

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

    auto repositories = Repositories();
    auto start_symbols = OptionalNonTerminals();
    auto derivation_rules = DerivationRuleLists();
    auto substitution_rules = SubstitutionRuleLists();

    auto visitor = EliminateRulesWithIdenticalBodyNonTerminalVisitor(repositories, start_symbols, derivation_rules, substitution_rules, substitution_map);

    grammar.accept(visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain());
}

using HanaSubstitutionNonTerminalOrderings =
    boost::hana::map<boost::hana::pair<boost::hana::type<ConceptTag>, std::unordered_map<NonTerminal<ConceptTag>, size_t>>,
                     boost::hana::pair<boost::hana::type<RoleTag>, std::unordered_map<NonTerminal<RoleTag>, size_t>>,
                     boost::hana::pair<boost::hana::type<BooleanTag>, std::unordered_map<NonTerminal<BooleanTag>, size_t>>,
                     boost::hana::pair<boost::hana::type<NumericalTag>, std::unordered_map<NonTerminal<NumericalTag>, size_t>>>;

class OrderSubstitutionRuleVisitor : public CopyVisitor
{
private:
    const HanaSubstitutionNonTerminalOrderings& m_orderings;

public:
    OrderSubstitutionRuleVisitor(Repositories& repositories,
                                 OptionalNonTerminals& start_symbols,
                                 DerivationRuleLists& derivation_rules,
                                 SubstitutionRuleLists& substitution_rules,
                                 const HanaSubstitutionNonTerminalOrderings& orderings) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
        m_orderings(orderings)
    {
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_start_symbols_container(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      second.value()->accept(*this);
                                      boost::hana::at_key(m_start_symbols, key) = std::any_cast<NonTerminal<FeatureType>>(get_result());
                                  }
                              });

        boost::hana::for_each(
            grammar.get_derivation_rules(),
            [&](auto&& pair)
            {
                auto key = boost::hana::first(pair);
                auto second = boost::hana::second(pair);
                using FeatureType = typename decltype(+key)::type;

                std::sort(second.begin(), second.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_head()->get_name() < rhs->get_head()->get_name(); });

                for (const auto& rule : second)
                {
                    rule->accept(*this);
                    boost::hana::at_key(m_derivation_rules, key).push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                }
            });

        boost::hana::for_each(
            grammar.get_substitution_rules(),
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
                    boost::hana::at_key(m_substitution_rules, key).push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                }
            });
    }
};

static Grammar order_substitution_rules(const Grammar& grammar)
{
    auto orderings = HanaSubstitutionNonTerminalOrderings {};

    boost::hana::for_each(grammar.get_substitution_rules(),
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

                              auto top_sort = graphs::bgl::topological_sort(graphs::DirectionTaggedType(graph, graphs::ForwardTag {}));

                              for (size_t i = 0; i < top_sort.size(); ++i)
                              {
                                  const auto& vertex = graph.get_vertex(top_sort.at(i));
                                  const auto nonterminal = vertex.template get_property<0>();
                                  non_terminal_to_vertex.at(nonterminal) = i;
                              }

                              boost::hana::at_key(orderings, key) = std::move(non_terminal_to_vertex);
                          });

    auto repositories = cnf_grammar::Repositories();
    auto start_symbols = cnf_grammar::OptionalNonTerminals();
    auto derivation_rules = cnf_grammar::DerivationRuleLists();
    auto substitution_rules = cnf_grammar::SubstitutionRuleLists();

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

    void visit(DerivationRule<ConceptTag> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<RoleTag> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<BooleanTag> constructor) override { visit_impl(constructor); }

    void visit(DerivationRule<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(DerivationRule<D> constructor)
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        constructor->get_body()->accept(*this);
    }

    void visit(SubstitutionRule<ConceptTag> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<RoleTag> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<BooleanTag> constructor) override { visit_impl(constructor); }

    void visit(SubstitutionRule<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    void visit_impl(SubstitutionRule<D> constructor)
    {
        // Store the head nonterminal but do not visit the head non terminal.
        m_head_non_terminals.insert(constructor->get_head()->get_name());

        m_body_non_terminals.insert(constructor->get_body()->get_name());
    }

    void visit(NonTerminal<ConceptTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<RoleTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<BooleanTag> constructor) override { visit_impl(constructor); }

    void visit(NonTerminal<NumericalTag> constructor) override { visit_impl(constructor); }

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
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

class IsEpsilonRuleVisitor : public RecurseVisitor
{
protected:
    const std::unordered_set<std::string>& m_epsilon_nonterminals;

    bool m_result;

public:
    IsEpsilonRuleVisitor(const std::unordered_set<std::string>& epsilon_nonterminals) : m_epsilon_nonterminals(epsilon_nonterminals) {}

    void visit(ConceptBot constructor) override { m_result = false; }
    void visit(ConceptTop constructor) override { m_result = false; }
    void visit(ConceptAtomicState<StaticTag> constructor) override { m_result = false; }
    void visit(ConceptAtomicState<FluentTag> constructor) override { m_result = false; }
    void visit(ConceptAtomicState<DerivedTag> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<StaticTag> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<FluentTag> constructor) override { m_result = false; }
    void visit(ConceptAtomicGoal<DerivedTag> constructor) override { m_result = false; }
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
    void visit(RoleAtomicState<StaticTag> constructor) override { m_result = false; }
    void visit(RoleAtomicState<FluentTag> constructor) override { m_result = false; }
    void visit(RoleAtomicState<DerivedTag> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<StaticTag> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<FluentTag> constructor) override { m_result = false; }
    void visit(RoleAtomicGoal<DerivedTag> constructor) override { m_result = false; }
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

    void visit(BooleanAtomicState<StaticTag> constructor) override { m_result = false; }
    void visit(BooleanAtomicState<FluentTag> constructor) override { m_result = false; }
    void visit(BooleanAtomicState<DerivedTag> constructor) override { m_result = false; }
    void visit(BooleanNonempty<ConceptTag> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(BooleanNonempty<RoleTag> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }

    void visit(NumericalCount<ConceptTag> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(NumericalCount<RoleTag> constructor) override { m_result = m_epsilon_nonterminals.contains(constructor->get_nonterminal()->get_name()); }
    void visit(NumericalDistance constructor) override
    {
        m_result = m_epsilon_nonterminals.contains(constructor->get_left_concept()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_role()->get_name())
                   || m_epsilon_nonterminals.contains(constructor->get_right_concept()->get_name());
    }

    bool get_result() const { return m_result; }
};

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
bool is_epsilon_rule(DerivationRule<D> rule, const std::unordered_set<std::string>& epsilon_nonterminals)
{
    auto visitor = IsEpsilonRuleVisitor(epsilon_nonterminals);
    rule->get_body()->accept(visitor);
    return visitor.get_result();
}

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
bool is_epsilon_rule(SubstitutionRule<D> rule, const std::unordered_set<std::string>& epsilon_nonterminals)
{
    return epsilon_nonterminals.contains(rule->get_body()->get_name());
}

class EliminateEpsilonRuleVisitor : public CopyVisitor
{
private:
    const std::unordered_set<std::string>& m_epsilon_nonterminals;

    bool eliminated;

public:
    EliminateEpsilonRuleVisitor(Repositories& repositories,
                                OptionalNonTerminals& start_symbols,
                                DerivationRuleLists& derivation_rules,
                                SubstitutionRuleLists& substitution_rules,
                                const std::unordered_set<std::string>& epsilon_nonterminals) :
        CopyVisitor(repositories, start_symbols, derivation_rules, substitution_rules),
        m_epsilon_nonterminals(epsilon_nonterminals),
        eliminated(false)
    {
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_start_symbols_container(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  if (second.has_value())
                                  {
                                      second.value()->accept(*this);
                                      boost::hana::at_key(m_start_symbols, key) = std::any_cast<NonTerminal<FeatureType>>(get_result());
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  auto second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (is_epsilon_rule(rule, m_epsilon_nonterminals))
                                      {
                                          eliminated = true;
                                          continue;
                                      }

                                      rule->accept(*this);
                                      boost::hana::at_key(m_derivation_rules, key).push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                                  }
                              });

        boost::hana::for_each(grammar.get_substitution_rules(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  auto second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      if (is_epsilon_rule(rule, m_epsilon_nonterminals))
                                      {
                                          eliminated = true;
                                          continue;
                                      }

                                      rule->accept(*this);
                                      boost::hana::at_key(m_substitution_rules, key).push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                                  }
                              });
    }

    bool get_eliminated() const { return eliminated; }
};

std::pair<Grammar, bool> eliminate_epsilon_rules(const Grammar& grammar)
{
    auto [head_nonterminals, body_nonterminals] = collect_head_and_body_nonterminals(grammar);

    for (const auto& head_nonterminal : head_nonterminals)
    {
        body_nonterminals.erase(head_nonterminal);
    }

    auto repositories = cnf_grammar::Repositories();
    auto start_symbols = cnf_grammar::OptionalNonTerminals();
    auto derivation_rules = cnf_grammar::DerivationRuleLists();
    auto substitution_rules = cnf_grammar::SubstitutionRuleLists();
    auto visitor = EliminateEpsilonRuleVisitor(repositories, start_symbols, derivation_rules, substitution_rules, body_nonterminals);

    grammar.accept(visitor);

    return { Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), std::move(substitution_rules), grammar.get_domain()),
             visitor.get_eliminated() };
}

Grammar simplify(const Grammar& grammar)
{
    /* Step 1 Identify rules with same body */

    auto simplified_grammar = eliminate_rules_with_identical_body(grammar);

    /* Step 2 eliminate epsilon rules */

    bool eliminated;
    do
    {
        auto [simplified_grammar_, eliminated_] = eliminate_epsilon_rules(simplified_grammar);
        simplified_grammar = std::move(simplified_grammar_);
        eliminated = eliminated_;

    } while (eliminated);

    /* Step 3 order substitution rules in order of evaluation through topological sorting */

    simplified_grammar = order_substitution_rules(simplified_grammar);

    /* Step 4 remove rules of unreachable non-terminals */

    return simplified_grammar;
}
}
