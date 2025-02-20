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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

#include <variant>

namespace mimir::dl::grammar
{

////////////////////////////
/// Interface
////////////////////////////

template<ConceptOrRole D>
class ConstructorVisitor
{
};

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
    virtual void visit(ConceptIntersection constructor) = 0;
    virtual void visit(ConceptUnion constructor) = 0;
    virtual void visit(ConceptNegation constructor) = 0;
    virtual void visit(ConceptValueRestriction constructor) = 0;
    virtual void visit(ConceptExistentialQuantification constructor) = 0;
    virtual void visit(ConceptRoleValueMapContainment constructor) = 0;
    virtual void visit(ConceptRoleValueMapEquality constructor) = 0;
    virtual void visit(ConceptNominal constructor) = 0;
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
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
class ConstructorOrNonTerminalVisitor
{
public:
    virtual ~ConstructorOrNonTerminalVisitor() = default;

    virtual void visit(ConstructorOrNonTerminal<D> constructor) = 0;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class NonTerminalVisitor
{
public:
    virtual ~NonTerminalVisitor() = default;

    virtual void visit(NonTerminal<D> constructor) = 0;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class DerivationRuleVisitor
{
public:
    virtual ~DerivationRuleVisitor() = default;

    virtual void visit(DerivationRule<D> constructor) = 0;
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
/// Recursive Visitor
////////////////////////////

template<ConceptOrRole D>
class RecurseConstructorVisitor;

template<ConceptOrRole D>
class RecurseConstructorOrNonTerminalVisitor;

template<ConceptOrRole D>
class RecurseNonTerminalVisitor;

template<ConceptOrRole D>
class RecurseDerivationRuleVisitor;

class RecurseGrammarVisitor;

/**
 * Concept
 */

template<ConceptOrRole D>
class RecurseConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class RecurseConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    RecurseConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    RecurseConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    RecurseConstructorVisitor() = default;

    virtual void initialize(RecurseConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            RecurseConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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
};

/**
 * Role
 */

template<>
class RecurseConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    RecurseConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    RecurseConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    RecurseConstructorVisitor() = default;

    virtual void initialize(RecurseConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            RecurseConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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
};

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
class RecurseConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
protected:
    RecurseNonTerminalVisitor<D>* m_nonterminal_visitor;
    RecurseConstructorVisitor<D>* m_constructor_visitor;

public:
    RecurseConstructorOrNonTerminalVisitor() = default;

    virtual void initialize(RecurseNonTerminalVisitor<D>& nonterminal_visitor, RecurseConstructorVisitor<D>& constructor_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class RecurseNonTerminalVisitor : public NonTerminalVisitor<D>
{
public:
    RecurseNonTerminalVisitor() = default;

    void visit(NonTerminal<D> constructor) override;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class RecurseDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    RecurseNonTerminalVisitor<D>* m_nonterminal_visitor;
    RecurseConstructorOrNonTerminalVisitor<D>* m_constructor_or_nonterminal_visitor;

public:
    RecurseDerivationRuleVisitor() = default;

    virtual void initialize(RecurseNonTerminalVisitor<D>& nonterminal_visitor, RecurseConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor);

    void visit(DerivationRule<D> constructor) override;
};

/**
 * Grammar
 */

class RecurseGrammarVisitor : public GrammarVisitor
{
protected:
    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, RecurseNonTerminalVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, RecurseNonTerminalVisitor<Role>*>>
        m_start_symbol_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, RecurseDerivationRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, RecurseDerivationRuleVisitor<Role>*>>
        m_derivation_rule_visitor;

public:
    RecurseGrammarVisitor() = default;

    virtual void visit(const Grammar& grammar) override;

    virtual void initialize(RecurseNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                            RecurseNonTerminalVisitor<Role>& role_start_symbol_visitor,
                            RecurseDerivationRuleVisitor<Concept>& concept_rule_visitor,
                            RecurseDerivationRuleVisitor<Role>& role_rule_visitor);
};

////////////////////////////
/// Recursive Copy Visitor
////////////////////////////

template<ConceptOrRole D>
class CopyConstructorVisitor;

template<ConceptOrRole D>
class CopyConstructorOrNonTerminalVisitor;

template<ConceptOrRole D>
class CopyNonTerminalVisitor;

template<ConceptOrRole D>
class CopyDerivationRuleVisitor;

class CopyGrammarVisitor;

/**
 * Concept
 */

template<ConceptOrRole D>
class CopyConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class CopyConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    ConstructorRepositories& m_repositories;  ///< the container that stores the result
    Constructor<Concept> m_result;            ///< the result of a visitation

    CopyConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    CopyConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories);

    virtual void initialize(CopyConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            CopyConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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

    const Constructor<Concept>& get_result() const;
};

/**
 * Role
 */

template<>
class CopyConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    ConstructorRepositories& m_repositories;  ///< the container that stores the result
    Constructor<Role> m_result;               ///< the result of a visitation

    CopyConstructorOrNonTerminalVisitor<Concept>* m_concept_or_nonterminal_visitor;
    CopyConstructorOrNonTerminalVisitor<Role>* m_role_or_nonterminal_visitor;

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories);

    virtual void initialize(CopyConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                            CopyConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor);

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
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
class CopyConstructorOrNonTerminalVisitor : public ConstructorOrNonTerminalVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;  ///< the container that stores the result
    ConstructorOrNonTerminal<D> m_result;     ///< the result of a visitation

    CopyNonTerminalVisitor<D>* m_nonterminal_visitor;
    CopyConstructorVisitor<D>* m_constructor_visitor;

public:
    explicit CopyConstructorOrNonTerminalVisitor(ConstructorRepositories& repositories);

    virtual void initialize(CopyNonTerminalVisitor<D>& nonterminal_visitor, CopyConstructorVisitor<D>& constructor_visitor);

    void visit(ConstructorOrNonTerminal<D> constructor) override;

    const ConstructorOrNonTerminal<D>& get_result() const;
};

/**
 * NonTerminal
 */

template<ConceptOrRole D>
class CopyNonTerminalVisitor : public NonTerminalVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;  ///< the container that stores the result
    NonTerminal<D> m_result;                  ///< the result of a visitation

public:
    explicit CopyNonTerminalVisitor(ConstructorRepositories& repositories);

    void visit(NonTerminal<D> constructor) override;

    const NonTerminal<D>& get_result() const;
};

/**
 * DerivationRule
 */

template<ConceptOrRole D>
class CopyDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    ConstructorRepositories& m_repositories;  ///< the container that stores the result
    DerivationRule<D> m_result;               ///< the result of a visitation

    CopyNonTerminalVisitor<D>* m_nonterminal_visitor;
    CopyConstructorOrNonTerminalVisitor<D>* m_constructor_or_nonterminal_visitor;

public:
    explicit CopyDerivationRuleVisitor(ConstructorRepositories& repositories);

    virtual void initialize(CopyNonTerminalVisitor<D>& nonterminal_visitor, CopyConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor);

    virtual void visit(DerivationRule<D> constructor) override;

    const DerivationRule<D>& get_result() const;
};

class CopyGrammarVisitor : public GrammarVisitor
{
protected:
    ConstructorRepositories& m_repositories;      ///< the container that stores the result
    StartSymbolsContainer m_start_symbols;        ///< the result of a visitation
    DerivationRulesContainer m_derivation_rules;  ///< the result of a visitation

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, CopyNonTerminalVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, CopyNonTerminalVisitor<Role>*>>
        m_start_symbol_visitor;

    boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, CopyDerivationRuleVisitor<Concept>*>,
                     boost::hana::pair<boost::hana::type<Role>, CopyDerivationRuleVisitor<Role>*>>
        m_derivation_rule_visitor;

public:
    explicit CopyGrammarVisitor(ConstructorRepositories& repositories);

    virtual void visit(const Grammar& grammar) override;

    virtual void initialize(CopyNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                            CopyNonTerminalVisitor<Role>& role_start_symbol_visitor,
                            CopyDerivationRuleVisitor<Concept>& concept_rule_visitor,
                            CopyDerivationRuleVisitor<Role>& role_rule_visitor);

    StartSymbolsContainer get_result_start_symbols();        ///< this move the result out
    DerivationRulesContainer get_result_derivation_rules();  ///< this move the result out
};

}

#endif
