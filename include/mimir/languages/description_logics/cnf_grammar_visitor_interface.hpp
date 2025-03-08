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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_VISITOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/cnf_grammar_containers.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::dl::cnf_grammar
{

////////////////////////////
/// Interface
////////////////////////////

/**
 * Constructors
 */

template<FeatureCategory D>
class ConstructorVisitor
{
};

/**
 * Concept
 */

template<>
class ConstructorVisitor<Concept>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(ConceptBot constructor) = 0;
    virtual void visit(ConceptTop constructor) = 0;
    virtual void visit(ConceptAtomicState<Static> constructor) = 0;
    virtual void visit(ConceptAtomicState<Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicState<Derived> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Static> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<Derived> constructor) = 0;
    virtual void visit(ConceptNominal constructor) = 0;
    virtual void visit(ConceptIntersection constructor) = 0;
    virtual void visit(ConceptUnion constructor) = 0;
    virtual void visit(ConceptNegation constructor) = 0;
    virtual void visit(ConceptValueRestriction constructor) = 0;
    virtual void visit(ConceptExistentialQuantification constructor) = 0;
    virtual void visit(ConceptRoleValueMapContainment constructor) = 0;
    virtual void visit(ConceptRoleValueMapEquality constructor) = 0;
};

/**
 * Role
 */

template<>
class ConstructorVisitor<Role>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(RoleUniversal constructor) = 0;
    virtual void visit(RoleAtomicState<Static> constructor) = 0;
    virtual void visit(RoleAtomicState<Fluent> constructor) = 0;
    virtual void visit(RoleAtomicState<Derived> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Static> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Fluent> constructor) = 0;
    virtual void visit(RoleAtomicGoal<Derived> constructor) = 0;
    virtual void visit(RoleIntersection constructor) = 0;
    virtual void visit(RoleUnion constructor) = 0;
    virtual void visit(RoleComplement constructor) = 0;
    virtual void visit(RoleInverse constructor) = 0;
    virtual void visit(RoleComposition constructor) = 0;
    virtual void visit(RoleTransitiveClosure constructor) = 0;
    virtual void visit(RoleReflexiveTransitiveClosure constructor) = 0;
    virtual void visit(RoleRestriction constructor) = 0;
    virtual void visit(RoleIdentity constructor) = 0;
};

/**
 * NonTerminal
 */

template<FeatureCategory D>
class NonTerminalVisitor
{
public:
    virtual ~NonTerminalVisitor() = default;

    virtual void visit(NonTerminal<D> constructor) = 0;
};

/**
 * DerivationRule
 */

template<FeatureCategory D>
class DerivationRuleVisitor
{
public:
    virtual ~DerivationRuleVisitor() = default;

    virtual void visit(DerivationRule<D> rule) = 0;
};

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
class SubstitutionRuleVisitor
{
public:
    virtual ~SubstitutionRuleVisitor() = default;

    virtual void visit(SubstitutionRule<D> rule) = 0;
};

/**
 * Grammar
 */

class GrammarVisitor
{
public:
    virtual ~GrammarVisitor() = default;

    virtual void visit(const Grammar& grammar) = 0;
};

////////////////////////////
/// Copy
////////////////////////////

template<FeatureCategory D>
class CopyConstructorVisitor;

template<FeatureCategory D>
class CopyNonTerminalVisitor;

template<FeatureCategory D>
class CopyDerivationRuleVisitor;

template<FeatureCategory D>
class CopySubstitutionRuleVisitor;

class CopyGrammarVisitor;

/**
 * Constructors
 */

template<FeatureCategory D>
class CopyConstructorVisitor : public ConstructorVisitor<D>
{
};

/**
 * Concept
 */

template<>
class CopyConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    ConstructorRepositories& m_repositories;
    CopyNonTerminalVisitor<Concept>& m_concept_visitor;
    CopyNonTerminalVisitor<Role>& m_role_visitor;

    Constructor<Concept> m_result;  ///< the result of a visitation

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories,
                                    CopyNonTerminalVisitor<Concept>& concept_visitor,
                                    CopyNonTerminalVisitor<Role>& role_visitor);

    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<Static> constructor) override;
    void visit(ConceptAtomicState<Fluent> constructor) override;
    void visit(ConceptAtomicState<Derived> constructor) override;
    void visit(ConceptAtomicGoal<Static> constructor) override;
    void visit(ConceptAtomicGoal<Fluent> constructor) override;
    void visit(ConceptAtomicGoal<Derived> constructor) override;
    void visit(ConceptNominal constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;

    const Constructor<Concept>& get_result() const;
};

/**
 * Role
 */

template<>
class CopyConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    ConstructorRepositories& m_repositories;
    CopyNonTerminalVisitor<Concept>& m_concept_visitor;
    CopyNonTerminalVisitor<Role>& m_role_visitor;

    Constructor<Role> m_result;  ///< the result of a visitation

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories,
                                    CopyNonTerminalVisitor<Concept>& concept_visitor,
                                    CopyNonTerminalVisitor<Role>& role_visitor);

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

    const Constructor<Role>& get_result() const;
};

/**
 * NonTerminal
 */

template<FeatureCategory D>
class CopyNonTerminalVisitor : public NonTerminalVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;

    NonTerminal<D> m_result;

public:
    explicit CopyNonTerminalVisitor(ConstructorRepositories& repositories);

    void visit(NonTerminal<D> constructor) override;

    const NonTerminal<D>& get_result() const;
};

/**
 * DerivationRule
 */

template<FeatureCategory D>
class CopyDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;
    CopyNonTerminalVisitor<D>& m_nonterminal_visitor;
    CopyConstructorVisitor<D>& m_constructor_visitor;

    DerivationRule<D> m_result;

public:
    explicit CopyDerivationRuleVisitor(ConstructorRepositories& repositories,
                                       CopyNonTerminalVisitor<D>& nonterminal_visitor,
                                       CopyConstructorVisitor<D>& constructor_visitor);

    void visit(DerivationRule<D> rule) override;

    const DerivationRule<D>& get_result() const;
};

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
class CopySubstitutionRuleVisitor : public SubstitutionRuleVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;
    CopyNonTerminalVisitor<D>& m_nonterminal_visitor;

    SubstitutionRule<D> m_result;

public:
    explicit CopySubstitutionRuleVisitor(ConstructorRepositories& repositories, CopyNonTerminalVisitor<D>& nonterminal_visitor);

    void visit(SubstitutionRule<D> rule) override;

    const SubstitutionRule<D>& get_result() const;
};

/**
 * Grammar
 */

class CopyGrammarVisitor : public GrammarVisitor
{
protected:
    ConstructorRepositories& m_repositories;
    StartSymbolsContainer& m_start_symbols;
    DerivationRulesContainer& m_derivation_rules;
    SubstitutionRulesContainer& m_substitution_rules;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, CopyNonTerminalVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, CopyNonTerminalVisitor<Role>*>>
        m_start_symbol_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, CopyDerivationRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, CopyDerivationRuleVisitor<Role>*>>
        m_derivation_rule_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, CopySubstitutionRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, CopySubstitutionRuleVisitor<Role>*>>
        m_substitution_rule_visitor;

public:
    CopyGrammarVisitor(ConstructorRepositories& repositories,
                       StartSymbolsContainer& start_symbols,
                       DerivationRulesContainer& derivation_rules,
                       SubstitutionRulesContainer& substitution_rules,
                       CopyNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                       CopyNonTerminalVisitor<Role>& role_start_symbol_visitor,
                       CopyDerivationRuleVisitor<Concept>& concept_derivation_rule_visitor,
                       CopyDerivationRuleVisitor<Role>& role_derivation_rule_visitor,
                       CopySubstitutionRuleVisitor<Concept>& concept_substitution_rule_visitor,
                       CopySubstitutionRuleVisitor<Role>& role_substitution_rule_visitor);

    void visit(const Grammar& grammar) override;
};

}

#endif
