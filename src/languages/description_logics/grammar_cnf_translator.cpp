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

template<FeatureCategory D>
class CollectNonTerminalsNonTerminalVisitor : public RecurseNonTerminalVisitor<D>
{
private:
    ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& m_nonterminal_map;

public:
    CollectNonTerminalsNonTerminalVisitor(ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& nonterminal_map) :
        m_nonterminal_map(nonterminal_map)
    {
    }

    void visit(NonTerminal<D> constructor) override
    {
        boost::hana::at_key(m_nonterminal_map, boost::hana::type<D> {}).emplace(constructor->get_name(), constructor);
    }
};

static ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical> collect_nonterminals_from_grammar(const Grammar& grammar)
{
    auto nonterminal_map = ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>();

    auto concept_visitor = RecurseConstructorVisitor<Concept>();
    auto role_visitor = RecurseConstructorVisitor<Role>();
    auto boolean_visitor = RecurseConstructorVisitor<Boolean>();
    auto numerical_visitor = RecurseConstructorVisitor<Numerical>();
    auto concept_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Concept>();
    auto role_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Role>();
    auto boolean_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Boolean>();
    auto numerical_or_nonterminal_visitor = RecurseConstructorOrNonTerminalVisitor<Numerical>();
    auto concept_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Concept>(nonterminal_map);
    auto role_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Role>(nonterminal_map);
    auto boolean_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Boolean>(nonterminal_map);
    auto numerical_nonterminal_visitor = CollectNonTerminalsNonTerminalVisitor<Numerical>(nonterminal_map);
    auto concept_derivation_rule_visitor = RecurseDerivationRuleVisitor<Concept>();
    auto role_derivation_rule_visitor = RecurseDerivationRuleVisitor<Role>();
    auto boolean_derivation_rule_visitor = RecurseDerivationRuleVisitor<Boolean>();
    auto numerical_derivation_rule_visitor = RecurseDerivationRuleVisitor<Numerical>();
    auto concept_start_nonterminal_visitor = RecurseNonTerminalVisitor<Concept>();
    auto role_start_nonterminal_visitor = RecurseNonTerminalVisitor<Role>();
    auto boolean_start_nonterminal_visitor = RecurseNonTerminalVisitor<Boolean>();
    auto numerical_start_nonterminal_visitor = RecurseNonTerminalVisitor<Numerical>();

    auto grammar_visitor = RecurseGrammarVisitor();

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_or_nonterminal_visitor),
                                                                       boost::hana::make_pair(boost::hana::type<Role> {}, &role_or_nonterminal_visitor));

    concept_visitor.initialize(concept_and_role_nonterminal_visitors);
    role_visitor.initialize(concept_and_role_nonterminal_visitors);
    boolean_visitor.initialize(concept_and_role_nonterminal_visitors);
    numerical_visitor.initialize(concept_and_role_nonterminal_visitors);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_visitor);

    auto nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_start_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Role> {}, &role_start_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Boolean> {}, &boolean_start_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Numerical> {}, &numerical_start_nonterminal_visitor));

    auto derivation_rule_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Role> {}, &role_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Boolean> {}, &boolean_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Numerical> {}, &numerical_derivation_rule_visitor));

    grammar_visitor.initialize(nonterminal_visitors, derivation_rule_visitors);

    grammar.accept(grammar_visitor);

    return nonterminal_map;
}

/**
 * Eliminate choice rules by introducing additional rules
 */

template<FeatureCategory D>
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
                                      m_start_symbols.insert(visitor.get_result());
                                  }
                              });

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      auto& visitor = *boost::hana::at_key(m_derivation_rule_visitor, key);
                                      rule->accept(visitor);
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
    auto boolean_visitor = CopyConstructorVisitor<Boolean>(repositories);
    auto numerical_visitor = CopyConstructorVisitor<Numerical>(repositories);
    auto concept_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Role>(repositories);
    auto boolean_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Boolean>(repositories);
    auto numerical_or_nonterminal_visitor = CopyConstructorOrNonTerminalVisitor<Numerical>(repositories);
    auto concept_nonterminal_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto boolean_nonterminal_visitor = CopyNonTerminalVisitor<Boolean>(repositories);
    auto numerical_nonterminal_visitor = CopyNonTerminalVisitor<Numerical>(repositories);
    auto concept_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Concept>(repositories, derivation_rules);
    auto role_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Role>(repositories, derivation_rules);
    auto boolean_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Boolean>(repositories, derivation_rules);
    auto numerical_derivation_rule_visitor = EliminateChoiceDerivationRuleVisitor<Numerical>(repositories, derivation_rules);
    auto concept_start_symbol_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_start_symbol_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto boolean_start_symbol_visitor = CopyNonTerminalVisitor<Boolean>(repositories);
    auto numerical_start_symbol_visitor = CopyNonTerminalVisitor<Numerical>(repositories);
    auto grammar_visitor = EliminateChoiceGrammarVisitor(repositories, start_symbols, derivation_rules);

    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    numerical_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor,
                               role_start_symbol_visitor,
                               boolean_start_symbol_visitor,
                               numerical_start_symbol_visitor,
                               concept_derivation_rule_visitor,
                               role_derivation_rule_visitor,
                               boolean_derivation_rule_visitor,
                               numerical_derivation_rule_visitor);

    grammar.accept(grammar_visitor);

    return Grammar(std::move(repositories), std::move(start_symbols), std::move(derivation_rules), grammar.get_domain());
}

/**
 * Eliminate nested constructors by introducing additional rules
 */

template<FeatureCategory D>
class EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor : public CopyConstructorOrNonTerminalVisitor<D>
{
private:
    DerivationRulesContainer& m_derivation_rules;
    ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& m_existing_nonterminals;

    size_t m_next_index;

    std::string get_free_nonterminal_name()
    {
        std::string candidate_name;
        do
        {
            candidate_name = "<" + D::name + "_" + std::to_string(m_next_index++) + ">";
        } while (boost::hana::at_key(m_existing_nonterminals, boost::hana::type<D> {}).contains(candidate_name));

        return candidate_name;
    }

public:
    EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor(ConstructorRepositories& repositories,
                                                                 DerivationRulesContainer& derivation_rules,
                                                                 ToNonTerminalMap<std::string, Concept, Role, Boolean, Numerical>& existing_nonterminals) :
        CopyConstructorOrNonTerminalVisitor<D>(repositories),
        m_derivation_rules(derivation_rules),
        m_existing_nonterminals(existing_nonterminals),
        m_next_index(0)
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

                    const auto non_terminal = this->m_repositories.template get_or_create_nonterminal<D>(get_free_nonterminal_name());

                    this->m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(non_terminal);

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

    auto concept_visitor = CopyConstructorVisitor<Concept>(repositories);
    auto role_visitor = CopyConstructorVisitor<Role>(repositories);
    auto boolean_visitor = CopyConstructorVisitor<Boolean>(repositories);
    auto numerical_visitor = CopyConstructorVisitor<Numerical>(repositories);
    auto concept_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Role>(repositories);
    auto boolean_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Boolean>(repositories);
    auto numerical_or_nonterminal_l1_visitor = CopyConstructorOrNonTerminalVisitor<Numerical>(repositories);
    auto concept_or_nonterminal_l2_visitor =
        EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Concept>(repositories, derivation_rules, nonterminal_map);
    auto role_or_nonterminal_l2_visitor = EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Role>(repositories, derivation_rules, nonterminal_map);
    auto boolean_or_nonterminal_l2_visitor =
        EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Boolean>(repositories, derivation_rules, nonterminal_map);
    auto numerical_or_nonterminal_l2_visitor =
        EliminateNestedConstructorsL2ConstructorOrNonTerminalVisitor<Numerical>(repositories, derivation_rules, nonterminal_map);
    auto concept_nonterminal_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto boolean_nonterminal_visitor = CopyNonTerminalVisitor<Boolean>(repositories);
    auto numerical_nonterminal_visitor = CopyNonTerminalVisitor<Numerical>(repositories);
    auto concept_derivation_rule_visitor = CopyDerivationRuleVisitor<Concept>(repositories);
    auto role_derivation_rule_visitor = CopyDerivationRuleVisitor<Role>(repositories);
    auto boolean_derivation_rule_visitor = CopyDerivationRuleVisitor<Boolean>(repositories);
    auto numerical_derivation_rule_visitor = CopyDerivationRuleVisitor<Numerical>(repositories);
    auto concept_start_symbol_visitor = CopyNonTerminalVisitor<Concept>(repositories);
    auto role_start_symbol_visitor = CopyNonTerminalVisitor<Role>(repositories);
    auto boolean_start_symbol_visitor = CopyNonTerminalVisitor<Boolean>(repositories);
    auto numerical_start_symbol_visitor = CopyNonTerminalVisitor<Numerical>(repositories);
    auto grammar_visitor = CopyGrammarVisitor(repositories, start_symbols, derivation_rules);

    concept_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    role_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    boolean_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    numerical_visitor.initialize(concept_or_nonterminal_l2_visitor, role_or_nonterminal_l2_visitor);
    concept_or_nonterminal_l1_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_l1_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_l1_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_l1_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);
    concept_or_nonterminal_l2_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_l2_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_l2_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_l2_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_l1_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_l1_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_l1_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_l1_visitor);
    grammar_visitor.initialize(concept_start_symbol_visitor,
                               role_start_symbol_visitor,
                               boolean_start_symbol_visitor,
                               numerical_start_symbol_visitor,
                               concept_derivation_rule_visitor,
                               role_derivation_rule_visitor,
                               boolean_derivation_rule_visitor,
                               numerical_derivation_rule_visitor);

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
    constructor->get_left_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto left_nonterminal = m_nonterminal_concept_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto right_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_left_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
    const auto left_nonterminal = m_nonterminal_concept_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*m_nonterminal_concept_visitor);
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
    constructor->get_left_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

cnf_grammar::Constructor<Concept> ToCNFConstructorVisitor<Concept>::get_result() const { return m_result; }

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
    constructor->get_left_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto right_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_role_intersection(left_nonterminal, right_nonterminal);
}
void ToCNFConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
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
    constructor->get_left_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
    const auto left_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_nonterminal_role_visitor);
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

cnf_grammar::Constructor<Role> ToCNFConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * Booleans
 */

ToCNFConstructorVisitor<Boolean>::ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories) {}

void ToCNFConstructorVisitor<Boolean>::initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& nonterminal_concept_visitor,
                                                  ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& nonterminal_role_visitor)
{
    m_nonterminal_concept_visitor = &nonterminal_concept_visitor;
    m_nonterminal_role_visitor = &nonterminal_role_visitor;
}

void ToCNFConstructorVisitor<Boolean>::visit(BooleanAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Boolean>::visit(BooleanAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Boolean>::visit(BooleanAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void ToCNFConstructorVisitor<Boolean>::visit(BooleanNonempty<Concept> constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_constructor_or_nonterminal()->accept(*m_nonterminal_concept_visitor);
    const auto concept_or_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_or_nonterminal);
}
void ToCNFConstructorVisitor<Boolean>::visit(BooleanNonempty<Role> constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_constructor_or_nonterminal()->accept(*m_nonterminal_role_visitor);
    const auto role_or_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(role_or_nonterminal);
}

cnf_grammar::Constructor<Boolean> ToCNFConstructorVisitor<Boolean>::get_result() const { return m_result; }

/**
 * Numericals
 */

ToCNFConstructorVisitor<Numerical>::ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories) {}

void ToCNFConstructorVisitor<Numerical>::initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& nonterminal_concept_visitor,
                                                    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& nonterminal_role_visitor)
{
    m_nonterminal_concept_visitor = &nonterminal_concept_visitor;
    m_nonterminal_role_visitor = &nonterminal_role_visitor;
}

void ToCNFConstructorVisitor<Numerical>::visit(NumericalCount<Concept> constructor)
{
    assert(m_nonterminal_concept_visitor);
    constructor->get_constructor_or_nonterminal()->accept(*m_nonterminal_concept_visitor);
    const auto concept_or_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_count(concept_or_nonterminal);
}
void ToCNFConstructorVisitor<Numerical>::visit(NumericalCount<Role> constructor)
{
    assert(m_nonterminal_role_visitor);
    constructor->get_constructor_or_nonterminal()->accept(*m_nonterminal_role_visitor);
    const auto role_or_nonterminal = m_nonterminal_role_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_count(role_or_nonterminal);
}
void ToCNFConstructorVisitor<Numerical>::visit(NumericalDistance constructor)
{
    assert(m_nonterminal_concept_visitor && m_nonterminal_role_visitor);
    constructor->get_left_concept_or_nonterminal()->accept(*m_nonterminal_concept_visitor);
    const auto left_concept_or_nonterminal = m_nonterminal_concept_visitor->get_result();
    constructor->get_role_or_nonterminal()->accept(*m_nonterminal_role_visitor);
    const auto role_or_nonterminal = m_nonterminal_role_visitor->get_result();
    constructor->get_right_concept_or_nonterminal()->accept(*m_nonterminal_concept_visitor);
    const auto right_concept_or_nonterminal = m_nonterminal_concept_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal);
}

cnf_grammar::Constructor<Numerical> ToCNFConstructorVisitor<Numerical>::get_result() const { return m_result; }

/**
 * ConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
ToCNFVariantConstructorOrNonTerminalVisitor<D>::ToCNFVariantConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) :
    m_repositories(repositories)
{
}

template<FeatureCategory D>
void ToCNFVariantConstructorOrNonTerminalVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor,
                                                                ToCNFConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<FeatureCategory D>
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

template<FeatureCategory D>
cnf_grammar::ConstructorOrNonTerminal<D> ToCNFVariantConstructorOrNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFVariantConstructorOrNonTerminalVisitor<Concept>;
template class ToCNFVariantConstructorOrNonTerminalVisitor<Role>;

template<FeatureCategory D>
ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::ToCNFNonTerminalConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) :
    m_repositories(repositories)
{
}

template<FeatureCategory D>
void ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
}

template<FeatureCategory D>
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

template<FeatureCategory D>
cnf_grammar::NonTerminal<D> ToCNFNonTerminalConstructorOrNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>;
template class ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>;

/**
 * NonTerminal
 */

template<FeatureCategory D>
ToCNFNonTerminalVisitor<D>::ToCNFNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<FeatureCategory D>
void ToCNFNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template<FeatureCategory D>
cnf_grammar::NonTerminal<D> ToCNFNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class ToCNFNonTerminalVisitor<Concept>;
template class ToCNFNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<FeatureCategory D>
ToCNFDerivationRuleVisitor<D>::ToCNFDerivationRuleVisitor(cnf_grammar::ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<FeatureCategory D>
void ToCNFDerivationRuleVisitor<D>::initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor,
                                               ToCNFVariantConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<FeatureCategory D>
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
            if constexpr (std::is_same_v<T, cnf_grammar::Constructor<D>>)
            {
                m_result = this->m_repositories.get_or_create_derivation_rule(head, arg);
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

template<FeatureCategory D>
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
                                     ToCNFNonTerminalVisitor<Boolean>& boolean_start_symbol_visitor,
                                     ToCNFNonTerminalVisitor<Numerical>& numerical_start_symbol_visitor,
                                     ToCNFDerivationRuleVisitor<Concept>& concept_rule_visitor,
                                     ToCNFDerivationRuleVisitor<Role>& role_rule_visitor,
                                     ToCNFDerivationRuleVisitor<Boolean>& boolean_rule_visitor,
                                     ToCNFDerivationRuleVisitor<Numerical>& numerical_rule_visitor)
{
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Concept> {}) = &concept_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Role> {}) = &role_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Boolean> {}) = &boolean_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Numerical> {}) = &numerical_start_symbol_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Concept> {}) = &concept_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Role> {}) = &role_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Boolean> {}) = &boolean_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Numerical> {}) = &numerical_rule_visitor;
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
                                  m_start_symbols.insert(visitor.get_result());
                              }
                          });

    boost::hana::for_each(
        grammar.get_derivation_rules_container().get(),
        [&](auto&& pair)
        {
            auto key = boost::hana::first(pair);
            const auto& second = boost::hana::second(pair);

            for (const auto& rule : second)
            {
                auto& visitor = *boost::hana::at_key(m_derivation_rule_visitor, key);
                rule->accept(visitor);
                const auto rule_variant = visitor.get_result();

                std::visit(
                    [&](auto&& arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, cnf_grammar::DerivationRule<Concept>> || std::is_same_v<T, cnf_grammar::DerivationRule<Role>>
                                      || std::is_same_v<T, cnf_grammar::DerivationRule<Boolean>> || std::is_same_v<T, cnf_grammar::DerivationRule<Numerical>>)
                        {
                            m_derivation_rules.push_back(arg);
                        }
                        else if constexpr (std::is_same_v<T, cnf_grammar::SubstitutionRule<Concept>> || std::is_same_v<T, cnf_grammar::SubstitutionRule<Role>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<Boolean>>
                                           || std::is_same_v<T, cnf_grammar::SubstitutionRule<Numerical>>)
                        {
                            m_substitution_rules.push_back(arg);
                        }
                        else
                        {
                            static_assert(dependent_false<T>::value,
                                          "ToCNFGrammarVisitor::visit(constructor): Missing implementation for DerivationOrSubstitutionRule type.");
                        }
                    },
                    rule_variant);
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
    auto boolean_constructor_visitor = ToCNFConstructorVisitor<Boolean>(repositories);
    auto numerical_constructor_visitor = ToCNFConstructorVisitor<Numerical>(repositories);
    auto concept_variant_concept_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_variant_role_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Role>(repositories);
    auto boolean_variant_concept_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Boolean>(repositories);
    auto numerical_variant_role_or_nonterminal_visitor = ToCNFVariantConstructorOrNonTerminalVisitor<Numerical>(repositories);
    auto concept_nonterminal_concept_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_role_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>(repositories);
    auto boolean_nonterminal_concept_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Boolean>(repositories);
    auto numerical_nonterminal_role_or_nonterminal_visitor = ToCNFNonTerminalConstructorOrNonTerminalVisitor<Numerical>(repositories);
    auto concept_nonterminal_visitor = ToCNFNonTerminalVisitor<Concept>(repositories);
    auto role_nonterminal_visitor = ToCNFNonTerminalVisitor<Role>(repositories);
    auto boolean_nonterminal_visitor = ToCNFNonTerminalVisitor<Boolean>(repositories);
    auto numerical_nonterminal_visitor = ToCNFNonTerminalVisitor<Numerical>(repositories);
    auto concept_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Concept>(repositories);
    auto role_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Role>(repositories);
    auto boolean_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Boolean>(repositories);
    auto numerical_derivation_rule_visitor = ToCNFDerivationRuleVisitor<Numerical>(repositories);
    auto grammar_visitor = ToCNFGrammarVisitor(repositories, start_symbols, derivation_rules, substitution_rules);

    concept_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    role_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    boolean_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    numerical_constructor_visitor.initialize(concept_nonterminal_concept_or_nonterminal_visitor, role_nonterminal_role_or_nonterminal_visitor);
    concept_variant_concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_constructor_visitor);
    role_variant_role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_constructor_visitor);
    boolean_variant_concept_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_constructor_visitor);
    numerical_variant_role_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_constructor_visitor);
    concept_nonterminal_concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor);
    role_nonterminal_role_or_nonterminal_visitor.initialize(role_nonterminal_visitor);
    boolean_nonterminal_concept_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor);
    numerical_nonterminal_role_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_variant_concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_variant_role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_variant_concept_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_variant_role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_nonterminal_visitor,
                               role_nonterminal_visitor,
                               boolean_nonterminal_visitor,
                               numerical_nonterminal_visitor,
                               concept_derivation_rule_visitor,
                               role_derivation_rule_visitor,
                               boolean_derivation_rule_visitor,
                               numerical_derivation_rule_visitor);

    grammar_visitor.visit(grammar);

    return cnf_grammar::Grammar(std::move(repositories),
                                std::move(start_symbols),
                                std::move(derivation_rules),
                                std::move(substitution_rules),
                                grammar.get_domain());
}

cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar)
{
    auto translated_grammar = eliminate_choices_in_rules(grammar);

    translated_grammar = eliminate_nested_constructors(translated_grammar);

    return parse_cnf_grammar(translated_grammar);
}
}
