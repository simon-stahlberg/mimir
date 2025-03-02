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

#include "grammar_cnf_translator.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"

namespace mimir::dl::grammar
{

/**
 * Step 1: Collect all non terminals to assign novel names to newly generated ones.
 */

template<ConceptOrRole D>
class CollectNonTerminalsNonTerminalVisitor : public RecurseNonTerminalVisitor<D>
{
private:
    NonTerminalMap<std::string>& m_nonterminal_map;

public:
    CollectNonTerminalsNonTerminalVisitor(NonTerminalMap<std::string>& nonterminal_map) : m_nonterminal_map(nonterminal_map) {}

    void visit(NonTerminal<D> constructor) override { m_nonterminal_map.insert(constructor->get_name(), constructor); }
};

static NonTerminalMap<std::string> collect_nonterminals_from_grammar(const Grammar& grammar)
{
    auto nonterminal_map = NonTerminalMap<std::string>();

    auto concept_visitor = RecurseConstructorVisitor<Concept>();
    auto role_visitor = RecurseConstructorVisitor<Role>();
    auto concept_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Concept>();
    auto role_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Role>();
    auto concept_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Concept>(nonterminal_map);
    auto role_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Role>(nonterminal_map);
    auto concept_derivation_rule_visitor = RecurseDerivationRuleVisitor<Concept>();
    auto role_derivation_rule_visitor = RecurseDerivationRuleVisitor<Role>();
    auto concept_start_symbol_visitor = RecurseNonTerminalVisitor<Concept>();
    auto role_start_symbol_visitor = RecurseNonTerminalVisitor<Role>();
    auto grammar_visitor = RecurseGrammarVisitor();

    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor, role_start_symbol_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    grammar.accept(grammar_visitor);

    return nonterminal_map;
}

/**
 * Eliminate choice rules by introducing additional rules
 */

template<ConceptOrRole D>
struct EliminateChoiceDerivationRuleVisitor : public CopyDerivationRuleVisitor<D>
{
    DerivationRulesContainer& m_derivation_rules;

    EliminateChoiceDerivationRuleVisitor(ConstructorRepositories& repositories, DerivationRulesContainer& derivation_rules) :
        CopyDerivationRuleVisitor<D>(repositories),
        m_derivation_rules(derivation_rules)
    {
    }

    void visit(DerivationRule<D> constructor) override
    {
        assert(this->m_nonterminal_visitor && this->m_constructor_or_nonterminal_visitor);

        constructor->get_non_terminal()->accept(*this->m_nonterminal_visitor);
        const auto copied_non_terminal = this->m_nonterminal_visitor->get_result();

        for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
        {
            constructor_or_nonterminal->accept(*this->m_constructor_or_nonterminal_visitor);

            this->m_derivation_rules.insert(this->m_repositories.template get_or_create_derivation_rule<D>(
                copied_non_terminal,
                ConstructorOrNonTerminalList<D> { this->m_constructor_or_nonterminal_visitor->get_result() }));
        }
    }
};

class EliminateChoiceGrammarVisitor : public CopyGrammarVisitor
{
public:
    EliminateChoiceGrammarVisitor(ConstructorRepositories& repositories, StartSymbolsContainer& start_symbols, DerivationRulesContainer& derivation_rules) :
        CopyGrammarVisitor(repositories, start_symbols, derivation_rules)
    {
    }

    void visit(const Grammar& grammar) override
    {
        boost::hana::for_each(grammar.get_start_symbols_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);

                                  if (second.has_value())
                                  {
                                      auto& visitor = *boost::hana::at_key(m_start_symbol_visitor, key);
                                      second.value()->accept(visitor);
                                      boost::hana::at_key(m_start_symbols.get(), key) = visitor.get_result();
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& non_terminal_and_rules : second)
                                  {
                                      const auto& [non_terminal, rules] = non_terminal_and_rules;

                                      for (const auto& rule : rules)
                                      {
                                          auto& visitor = *boost::hana::at_key(m_derivation_rule_visitor, key);
                                          rule->accept(visitor);
                                      }
                                  }
                              });
    }
};

static Grammar eliminate_choices_in_rules(const Grammar& grammar)
{
    auto repositories = ConstructorRepositories();
    auto start_symbols = StartSymbolsContainer();
    auto derivation_rules = DerivationRulesContainer();

    auto concept_visitor = CopyConstructorVisitor<Concept>(repositories);
    auto role_visitor = CopyConstructorVisitor<Role>(repositories);
    auto concept_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Role>(repositories);
    auto concept_nonterminal_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto concept_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Concept>(repositories, derivation_rules);
    auto role_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Role>(repositories, derivation_rules);
    auto concept_start_symbol_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_start_symbol_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto grammar_visitor = EliminateChoiceGrammarVisitor(repositories, start_symbols, derivation_rules);

    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor, role_start_symbol_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    grammar.accept(grammar_visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), grammar.get_domain());
}

/**
 * Eliminate nested constructors by introducing additional rules
 */

template<ConceptOrRole D>
class EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor : public CopyConstructorOrNonTerminalVisitor<D>
{
private:
    DerivationRulesContainer& m_derivation_rules;
    NonTerminalMap<std::string>& m_existing_nonterminals;

    size_t m_next_index;
    std::unordered_map<Constructor<D>, ConstructorOrNonTerminal<D>> m_remapped;

    std::string get_free_nonterminal_name()
    {
        std::string candidate_name;
        do
        {
            candidate_name = "<" + D::name + "_" + std::to_string(m_next_index++) + ">";
        } while (m_existing_nonterminals.template contains<D>(candidate_name));

        return candidate_name;
    }

public:
    EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor(ConstructorRepositories& repositories,
                                                                 DerivationRulesContainer& derivation_rules,
                                                                 NonTerminalMap<std::string>& existing_nonterminals) :
        CopyConstructorOrNonTerminalVisitor<D>(repositories),
        m_derivation_rules(derivation_rules),
        m_existing_nonterminals(existing_nonterminals),
        m_next_index(0),
        m_remapped()
    {
    }

    void visit(ConstructorOrNonTerminal<D> constructor) override
    {
        std::visit(
            [this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Constructor<D>>)
                {
                    assert(this->m_constructor_visitor);
                    arg->accept(*this->m_constructor_visitor);

                    const auto it = m_remapped.find(arg);

                    if (it != m_remapped.end())
                    {
                        this->m_result = it->second;
                        return;
                    }

                    const auto non_terminal = this->m_repositories.template get_or_create_nonterminal<D>(get_free_nonterminal_name());

                    this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(non_terminal);

                    m_remapped.emplace(arg, this->m_result);

                    const auto constructor =
                        this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(this->m_constructor_visitor->get_result());

                    const auto derivation_rule =
                        this->m_repositories.template get_or_create_derivation_rule<D>(non_terminal, ConstructorOrNonTerminalList<D> { constructor });

                    this->m_derivation_rules.insert(derivation_rule);
                }
                else if constexpr (std::is_same_v<T, NonTerminal<D>>)
                {
                    assert(this->m_nonterminal_visitor);
                    arg->accept(*this->m_nonterminal_visitor);
                    this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(this->m_nonterminal_visitor->get_result());
                }
                else
                {
                    static_assert(dependent_false<D>::value,
                                  "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
                }
            },
            constructor->get_constructor_or_non_terminal());
    }
};

static Grammar eliminate_nested_constructors(const Grammar& grammar)
{
    auto repositories = ConstructorRepositories();
    auto start_symbols = StartSymbolsContainer();
    auto derivation_rules = DerivationRulesContainer();

    auto nonterminal_map = collect_nonterminals_from_grammar(grammar);

    // mimir::operator<<(std::cout, nonterminal_map.get());
    // std::cout << std::endl;

    auto concept_visitor = CopyConstructorVisitor<Concept>(repositories);
    auto role_visitor = CopyConstructorVisitor<Role>(repositories);
    auto concept_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Role>(repositories);
    auto concept_or_nonterminal_l2_visitor =
        EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Concept>(repositories, derivation_rules, nonterminal_map);
    auto role_or_nonterminal_l2_visitor = EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Role>(repositories, derivation_rules, nonterminal_map);
    auto concept_nonterminal_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto concept_derivation_rule_visitor = CopyDerivationRuleVisitor<Concept>(repositories);
    auto role_derivation_rule_visitor = CopyDerivationRuleVisitor<Role>(repositories);
    auto concept_start_symbol_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_start_symbol_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto grammar_visitor = CopyGrammarVisitor(repositories, start_symbols, derivation_rules);

    concept_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    role_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    concept_or_nonterminal_l1_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_l1_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_or_nonterminal_l2_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_l2_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_l1_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_l1_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor, role_start_symbol_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    grammar.accept(grammar_visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), grammar.get_domain());
}

/**
 * ToCnfGrammar
 */

/**
 * Concept
 */

ToCNFConstructorVisitor<Concept>::ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories) {}

void ToCNFConstructorVisitor<Concept>::initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& nonterminal_concept_visitor,
                                                  ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& nonterminal_role_visitor)
{
    m_nonterminal_concept_visitor = &nonterminal_concept_visitor;
    m_nonterminal_role_visitor = &nonterminal_role_visitor;
}

void ToCNFConstructorVisitor<Concept>::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void ToCNFConstructorVisitor<Concept>::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_concept_or_non_terminal_left()->accept(*m_nonterminal_concept_visitor);
    const auto left_nonterminal = m_nonterminal_concept_visitor->get_result();
    constructor->get_concept_or_non_terminal_right()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_concept_or_non_terminal_left()->accept(*m_nonterminal_concept_visitor);
    const auto left_nonterminal = m_nonterminal_concept_visitor->get_result();
    constructor->get_concept_or_non_terminal_right()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_union(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_negation(nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    assert(m_nonterminal_concept_visitor && m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_value_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    assert(m_nonterminal_concept_visitor && m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_existential_quantification(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_role_or_non_terminal_right()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_role_or_non_terminal_right()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

cnf_grammar::ConstructorVariant<Concept> ToCNFConstructorVisitor<Concept>::get_result() const { return m_result; }

/**
 * Role
 */

ToCNFConstructorVisitor<Role>::ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories) {}

void ToCNFConstructorVisitor<Role>::initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& nonterminal_concept_visitor,
                                               ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& nonterminal_role_visitor)
{
    m_nonterminal_concept_visitor = &nonterminal_concept_visitor;
    m_nonterminal_role_visitor = &nonterminal_role_visitor;
}

void ToCNFConstructorVisitor<Role>::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void ToCNFConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_role_or_non_terminal_right()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_role_or_non_terminal_right()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_union(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_complement(nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_inverse(nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_role_or_non_terminal_right()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_composition(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_transitive_closure(nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    assert(m_nonterminal_concept_visitor && m_nonterminal_role_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_role_restriction(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_role_identity(nonterminal);
}

cnf_grammar::ConstructorVariant<Role> ToCNFConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
ToCNFVariantConstructorOrNonTerminalVisitor<D>::ToCNFVariantConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) :
    m_repositories(repositories)
{
}

template<ConceptOrRole D>
void ToCNFVariantConstructorOrNonTerminalVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor,
                                                                ToCNFConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<ConceptOrRole D>
void ToCNFVariantConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                assert(this->m_constructor_visitor);
                arg->accept(*this->m_constructor_visitor);
                m_result = this->m_constructor_visitor->get_result();
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                assert(this->m_nonterminal_visitor);
                arg->accept(*this->m_nonterminal_visitor);
                m_result = this->m_nonterminal_visitor->get_result();
            }
            else
            {
                static_assert(dependent_false<D>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template<ConceptOrRole D>
cnf_grammar::ConstructorVariantOrNonTerminal<D> ToCNFVariantConstructorOrNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFVariantConstructorOrNonTerminalVisitor<Concept>;
template class ToCNFVariantConstructorOrNonTerminalVisitor<Role>;

template<ConceptOrRole D>
ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::ToCNFNonTerminalConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) :
    m_repositories(repositories)
{
}

template<ConceptOrRole D>
void ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
}

template<ConceptOrRole D>
void ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                throw std::logic_error("ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::visit(constructor): Expected NonTerminal.");
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                assert(this->m_nonterminal_visitor);
                arg->accept(*this->m_nonterminal_visitor);
                m_result = this->m_nonterminal_visitor->get_result();
            }
            else
            {
                static_assert(dependent_false<D>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template<ConceptOrRole D>
cnf_grammar::NonTerminal<D> ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>;
template class ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>;

/**
 * NonTerminal
 */

template<ConceptOrRole D>
ToCNFNonTerminalVisitor<D>::ToCNFNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void ToCNFNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template<ConceptOrRole D>
cnf_grammar::NonTerminal<D> ToCNFNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFNonTerminalVisitor<Concept>;
template class ToCNFNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
ToCNFDerivationRuleVisitor<D>::ToCNFDerivationRuleVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void ToCNFDerivationRuleVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor,
                                               ToCNFVariantConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<ConceptOrRole D>
void ToCNFDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(this->m_nonterminal_visitor && this->m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);
    const auto head = m_nonterminal_visitor->get_result();

    if (constructor->get_constructor_or_non_terminals().size() != 1)
    {
        throw std::logic_error("ToCNFDerivationRuleVisitor<D>::visit(constructor): Expected single choice");
    }

    constructor->get_constructor_or_non_terminals().front()->accept(*m_constructor_or_nonterminal_visitor);
    const auto body_variant = m_constructor_or_nonterminal_visitor->get_result();

    std::visit(
        [&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, cnf_grammar::ConstructorVariant<D>>)
            {
                std::visit([&](auto&& arg2) { m_result = this->m_repositories.get_or_create_derivation_rule(head, arg2); }, arg);
            }
            else if constexpr (std::is_same_v<T, cnf_grammar::NonTerminal<D>>)
            {
                m_result = this->m_repositories.get_or_create_substitution_rule(head, arg);
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "ToCNFDerivationRuleVisitor<T>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        body_variant);
}

template<ConceptOrRole D>
cnf_grammar::DerivationOrSubstitutionRule<D> ToCNFDerivationRuleVisitor<D>::get_result() const
{
    return m_result;
}

/**
 * Grammar
 */

ToCNFGrammarVisitor::ToCNFGrammarVisitor(cnf_grammar::ConstructorRepositories& repositories,
                                         cnf_grammar::StartSymbolsContainer& start_symbols,
                                         cnf_grammar::DerivationRulesContainer& derivation_rules,
                                         cnf_grammar::SubstitutionRulesContainer& substitution_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules),
    m_substitution_rules(substitution_rules)
{
}

void ToCNFGrammarVisitor::initialize(ToCNFNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                                     ToCNFNonTerminalVisitor<Role>& role_start_symbol_visitor,
                                     ToCNFDerivationRuleVisitor<Concept>& concept_rule_visitor,
                                     ToCNFDerivationRuleVisitor<Role>& role_rule_visitor)
{
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Concept> {}) = &concept_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Role> {}) = &role_start_symbol_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Concept> {}) = &concept_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Role> {}) = &role_rule_visitor;
}

void ToCNFGrammarVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              if (second.has_value())
                              {
                                  auto& visitor = *boost::hana::at_key(m_start_symbol_visitor, key);
                                  second.value()->accept(visitor);
                                  boost::hana::at_key(m_start_symbols.get(), key) = visitor.get_result();
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& non_terminal_and_rules : second)
                              {
                                  const auto& [non_terminal, rules] = non_terminal_and_rules;

                                  for (const auto& rule : rules)
                                  {
                                      auto& visitor = *boost::hana::at_key(m_derivation_rule_visitor, key);
                                      rule->accept(visitor);
                                      const auto rule_variant = visitor.get_result();

                                      std::visit(
                                          [&](auto&& arg)
                                          {
                                              using T = std::decay_t<decltype(arg)>;
                                              if constexpr (std::is_same_v<T, cnf_grammar::DerivationRule<Concept, cnf_grammar::Primitive>>
                                                            || std::is_same_v<T, cnf_grammar::DerivationRule<Concept, cnf_grammar::Composite>>
                                                            || std::is_same_v<T, cnf_grammar::DerivationRule<Role, cnf_grammar::Primitive>>
                                                            || std::is_same_v<T, cnf_grammar::DerivationRule<Role, cnf_grammar::Composite>>)
                                              {
                                                  m_derivation_rules.insert(arg);
                                              }
                                              else if constexpr (std::is_same_v<T, cnf_grammar::SubstitutionRule<Concept>>
                                                                 || std::is_same_v<T, cnf_grammar::SubstitutionRule<Role>>)
                                              {
                                                  m_substitution_rules.insert(arg);
                                              }
                                              else
                                              {
                                                  static_assert(
                                                      dependent_false<T>::value,
                                                      "ToCNFGrammarVisitor::visit(constructor): Missing implementation for DerivationOrSubstitutionRule type.");
                                              }
                                          },
                                          rule_variant);
                                  }
                              }
                          });
}

static cnf_grammar::Grammar parse_cnf_grammar(const Grammar& grammar)
{
    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();

    auto concept_constructor_visitor = ToCNFConstructorVisitor<Concept>(repositories);
    auto role_constructor_visitor = ToCNFConstructorVisitor<Role>(repositories);
    auto concept_variant_concept_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_variant_role_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Role>(repositories);
    auto concept_nonterminal_concept_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_role_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>(repositories);
    auto concept_nonterminal_visitor = ToCNFNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = ToCNFNonTerminalVisitor<Role>(repositories);
    auto concept_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Concept>(repositories);
    auto role_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Role>(repositories);
    auto grammar_visitor = ToCNFGrammarVisitor(repositories, start_symbols, derivation_rules, substitution_rules);

    concept_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    role_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    concept_variant_concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_constructor_visitor);
    role_variant_role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_constructor_visitor);
    concept_nonterminal_concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor);
    role_nonterminal_role_or_nonterminal_visitor.initialize(role_nonterminal_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_variant_concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_variant_role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_nonterminal_visitor, role_nonterminal_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    grammar_visitor.visit(grammar);

    return cnf_grammar::Grammar(std::move(repositories),
                                std::move(start_symbols),
                                std::move(derivation_rules),
                                std::move(substitution_rules),
                                grammar.get_domain());
}

cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar)
{
    std::cout << "translate_to_cnf" << std::endl;

    std::cout << grammar << std::endl;

    auto translated_grammar = eliminate_choices_in_rules(grammar);
    std::cout << translated_grammar << std::endl;

    translated_grammar = eliminate_nested_constructors(translated_grammar);
    std::cout << translated_grammar << std::endl;

    return parse_cnf_grammar(translated_grammar);
}
}
