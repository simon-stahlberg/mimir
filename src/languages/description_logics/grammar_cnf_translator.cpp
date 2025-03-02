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
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

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

template<ConceptOrRole D>
class ToCNFConstructorVisitor;

template<ConceptOrRole D>
class ToCNFConstructorOrNonTerminalVisitor;

template<ConceptOrRole D>
class ToCNFNonTerminalVisitor;

template<ConceptOrRole D>
class ToCNFDerivationRuleVisitor;

class ToCNFGrammarVisitor;
}

namespace mimir::dl::cnf_grammar
{
template<ConceptOrRole D>
using ConstructorVariant = std::variant<Constructor<D, Primitive>, Constructor<D, Composite>>;

template<ConceptOrRole D>
using ConstructorVariantOrNonTerminal = std::variant<ConstructorVariant<D>, NonTerminal<D>>;

template<ConceptOrRole D>
using DerivationOrSubstitutionRule = std::variant<ConstructorVariant<D>, SubstitutionRule<D>>;
}

namespace mimir::dl::grammar
{
/**
 * Concept
 */

template<ConceptOrRole D>
class ToCNFConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class ToCNFConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::ConstructorVariant<Concept> m_result;  ///< the result of a visitation

    ToCNFConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    ToCNFConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    explicit ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories);

    virtual void initialize(ToCNFConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            ToCNFConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<Static> constructor) override;
    void visit(ConceptAtomicState<Fluent> constructor) override;
    void visit(ConceptAtomicState<Derived> constructor) override;
    void visit(ConceptAtomicGoal<Static> constructor) override;
    void visit(ConceptAtomicGoal<Fluent> constructor) override;
    void visit(ConceptAtomicGoal<Derived> constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    void visit(ConceptNominal constructor) override;

    cnf_grammar::ConstructorVariant<Concept> get_result() const;
};

/**
 * Role
 */

template<>
class ToCNFConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::ConstructorVariant<Role> m_result;  ///< the result of a visitation

    ToCNFConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    ToCNFConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    explicit ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories);

    virtual void initialize(ToCNFConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            ToCNFConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<Static> constructor) override;
    void visit(RoleAtomicState<Fluent> constructor) override;
    void visit(RoleAtomicState<Derived> constructor) override;
    void visit(RoleAtomicGoal<Static> constructor) override;
    void visit(RoleAtomicGoal<Fluent> constructor) override;
    void visit(RoleAtomicGoal<Derived> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;

    cnf_grammar::ConstructorVariant<Role> get_result() const;
};

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
class ToCNFConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::ConstructorVariantOrNonTerminal<D> m_result;  ///< the result of a visitation

    ToCNFNonTerminalVisitor<D>* m_nonterminal_visitor;
    ToCNFConstructorVisitor<D>* m_constructor_visitor;

public:
    explicit ToCNFConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories);

    virtual void initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor, ToCNFConstructorVisitor<D>& constructor_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;

    cnf_grammar::ConstructorVariantOrNonTerminal<D> get_result() const;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class ToCNFNonTerminalVisitor : public NonTerminalVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::NonTerminal<D> m_result;  ///< the result of a visitation

public:
    explicit ToCNFNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void visit(NonTerminal<D> constructor) override;

    cnf_grammar::NonTerminal<D> get_result() const;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class ToCNFDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::DerivationOrSubstitutionRule<D> m_result;  ///< the result of a visitation

    ToCNFNonTerminalVisitor<D>* m_nonterminal_visitor;
    ToCNFConstructorOrNonTerminalVisitor<D>* m_constructor_or_nonterminal_visitor;

public:
    explicit ToCNFDerivationRuleVisitor(cnf_grammar::ConstructorRepositories& repositories);

    virtual void initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor, ToCNFConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor);

    virtual void visit(DerivationRule<D> constructor) override;

    cnf_grammar::DerivationOrSubstitutionRule<D> get_result() const;
};

class ToCNFGrammarVisitor : public GrammarVisitor
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::StartSymbolsContainer& m_start_symbols;
    cnf_grammar::DerivationRulesContainer& m_derivation_rules;
    cnf_grammar::SubstitutionRulesContainer& m_substitution_rules;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, ToCNFNonTerminalVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, ToCNFNonTerminalVisitor<Role>*>>
        m_start_symbol_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, ToCNFDerivationRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, ToCNFDerivationRuleVisitor<Role>*>>
        m_derivation_rule_visitor;

public:
    ToCNFGrammarVisitor(cnf_grammar::ConstructorRepositories& repositories,
                        cnf_grammar::StartSymbolsContainer& start_symbols,
                        cnf_grammar::DerivationRulesContainer& derivation_rules,
                        cnf_grammar::SubstitutionRulesContainer& substitution_rules) :
        m_repositories(repositories),
        m_start_symbols(start_symbols),
        m_derivation_rules(derivation_rules),
        m_substitution_rules(substitution_rules)
    {
    }

    void visit(const Grammar& grammar) override {}

    void initialize(ToCNFNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                    ToCNFNonTerminalVisitor<Role>& role_start_symbol_visitor,
                    ToCNFDerivationRuleVisitor<Concept>& concept_rule_visitor,
                    ToCNFDerivationRuleVisitor<Role>& role_rule_visitor)
    {
    }
};

static cnf_grammar::Grammar parse_cnf_grammar(const Grammar& grammar)
{
    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();
}

cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar)
{
    std::cout << "translate_to_cnf" << std::endl;

    std::cout << grammar << std::endl;

    auto translated_grammar = eliminate_choices_in_rules(grammar);
    std::cout << translated_grammar << std::endl;

    translated_grammar = eliminate_nested_constructors(translated_grammar);
    std::cout << translated_grammar << std::endl;

    exit(1);

    // return parse_cnf_grammar(translated_grammar);
}
}
