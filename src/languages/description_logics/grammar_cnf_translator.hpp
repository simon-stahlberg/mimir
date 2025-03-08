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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CNF_TRANSLATOR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CNF_TRANSLATOR_HPP_

#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

namespace mimir::dl::cnf_grammar
{
template<FeatureCategory D>
using ConstructorOrNonTerminal = std::variant<Constructor<D>, NonTerminal<D>>;

template<FeatureCategory D>
using DerivationOrSubstitutionRule = std::variant<DerivationRule<D>, SubstitutionRule<D>>;
}

namespace mimir::dl::grammar
{

template<FeatureCategory D>
class ToCNFConstructorVisitor;

template<FeatureCategory D>
class ToCNFVariantConstructorOrNonTerminalVisitor;

template<FeatureCategory D>
class ToCNFNonTerminalConstructorOrNonTerminalVisitor;

template<FeatureCategory D>
class ToCNFNonTerminalVisitor;

template<FeatureCategory D>
class ToCNFDerivationRuleVisitor;

class ToCNFGrammarVisitor;

template<FeatureCategory D>
class ToCNFConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class ToCNFConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::Constructor<Concept> m_result;  ///< the result of a visitation

    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>* m_nonterminal_concept_visitor;
    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>* m_nonterminal_role_visitor;

public:
    explicit ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& nonterminal_concept_visitor,
                    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& nonterminal_role_visitor);

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

    cnf_grammar::Constructor<Concept> get_result() const;
};

/**
 * Role
 */

template<>
class ToCNFConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::Constructor<Role> m_result;  ///< the result of a visitation

    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>* m_nonterminal_concept_visitor;
    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>* m_nonterminal_role_visitor;

public:
    explicit ToCNFConstructorVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void initialize(ToCNFNonTerminalConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                    ToCNFNonTerminalConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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

    cnf_grammar::Constructor<Role> get_result() const;
};

/**
 * ConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
class ToCNFVariantConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::ConstructorOrNonTerminal<D> m_result;  ///< the result of a visitation

    ToCNFNonTerminalVisitor<D>* m_nonterminal_visitor;
    ToCNFConstructorVisitor<D>* m_constructor_visitor;

public:
    explicit ToCNFVariantConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor, ToCNFConstructorVisitor<D>& constructor_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;

    cnf_grammar::ConstructorOrNonTerminal<D> get_result() const;
};

template<FeatureCategory D>
class ToCNFNonTerminalConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::NonTerminal<D> m_result;  ///< the result of a visitation

    ToCNFNonTerminalVisitor<D>* m_nonterminal_visitor;

public:
    explicit ToCNFNonTerminalConstructorOrNonTerminalVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;

    cnf_grammar::NonTerminal<D> get_result() const;
};

/**
 * NonTerminal
 */

template<FeatureCategory D>
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

template<FeatureCategory D>
class ToCNFDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    cnf_grammar::ConstructorRepositories& m_repositories;
    cnf_grammar::DerivationOrSubstitutionRule<D> m_result;  ///< the result of a visitation

    ToCNFNonTerminalVisitor<D>* m_nonterminal_visitor;
    ToCNFVariantConstructorOrNonTerminalVisitor<D>* m_constructor_or_nonterminal_visitor;

public:
    explicit ToCNFDerivationRuleVisitor(cnf_grammar::ConstructorRepositories& repositories);

    void initialize(ToCNFNonTerminalVisitor<D>& nonterminal_visitor, ToCNFVariantConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor);

    void visit(DerivationRule<D> constructor) override;

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
                        cnf_grammar::SubstitutionRulesContainer& substitution_rules);

    void visit(const Grammar& grammar) override;

    void initialize(ToCNFNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                    ToCNFNonTerminalVisitor<Role>& role_start_symbol_visitor,
                    ToCNFDerivationRuleVisitor<Concept>& concept_rule_visitor,
                    ToCNFDerivationRuleVisitor<Role>& role_rule_visitor);
};

extern cnf_grammar::Grammar translate_to_cnf(const Grammar& grammar);
}

#endif
