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
 * Booleans
 */

template<>
class ConstructorVisitor<Boolean>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(BooleanAtomicState<Static> constructor) = 0;
    virtual void visit(BooleanAtomicState<Fluent> constructor) = 0;
    virtual void visit(BooleanAtomicState<Derived> constructor) = 0;
    virtual void visit(BooleanNonempty<Concept> constructor) = 0;
    virtual void visit(BooleanNonempty<Role> constructor) = 0;
};

/**
 * Numericals
 */

template<>
class ConstructorVisitor<Numerical>
{
public:
    virtual ~ConstructorVisitor() = default;

    virtual void visit(NumericalCount<Concept> constructor) = 0;
    virtual void visit(NumericalCount<Role> constructor) = 0;
    virtual void visit(NumericalDistance constructor) = 0;
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
/// Recurse
////////////////////////////

template<FeatureCategory D>
class RecurseConstructorVisitor;
template<FeatureCategory... Ds>
using HanaRecurseConstructorVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<RecurseConstructorVisitor<Ds>>>...>;

template<FeatureCategory D>
class RecurseNonTerminalVisitor;
template<FeatureCategory... Ds>
using HanaRecurseNonTerminalVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<RecurseNonTerminalVisitor<Ds>>>...>;

template<FeatureCategory D>
class RecurseDerivationRuleVisitor;
template<FeatureCategory... Ds>
using HanaRecurseDerivationRuleVisitors =
    boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<RecurseDerivationRuleVisitor<Ds>>>...>;

template<FeatureCategory D>
class RecurseSubstitutionRuleVisitor;
template<FeatureCategory... Ds>
using HanaRecurseSubstitutionRuleVisitors =
    boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<RecurseSubstitutionRuleVisitor<Ds>>>...>;

template<FeatureCategory D, IsHanaMap M>
auto& get_visitor(M& map)
{
    return boost::hana::at_key(map, boost::hana::type<D> {}).get();
}

class RecurseGrammarVisitor;

/**
 * Constructors
 */

template<FeatureCategory D>
class RecurseConstructorVisitor : public ConstructorVisitor<D>
{
};

/**
 * Concept
 */

template<>
class RecurseConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    HanaRecurseNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

public:
    RecurseConstructorVisitor(HanaRecurseNonTerminalVisitors<Concept, Role> nonterminal_visitors);

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
};

/**
 * Role
 */

template<>
class RecurseConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    HanaRecurseNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

public:
    explicit RecurseConstructorVisitor(HanaRecurseNonTerminalVisitors<Concept, Role> nonterminal_visitors);

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
 * Booleans
 */

template<>
class RecurseConstructorVisitor<Boolean> : public ConstructorVisitor<Boolean>
{
protected:
    HanaRecurseNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

public:
    explicit RecurseConstructorVisitor(HanaRecurseNonTerminalVisitors<Concept, Role> nonterminal_visitors);

    void visit(BooleanAtomicState<Static> constructor) override;
    void visit(BooleanAtomicState<Fluent> constructor) override;
    void visit(BooleanAtomicState<Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;
};

/**
 * Numericals
 */

template<>
class RecurseConstructorVisitor<Numerical> : public ConstructorVisitor<Numerical>
{
protected:
    HanaRecurseNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

public:
    explicit RecurseConstructorVisitor(HanaRecurseNonTerminalVisitors<Concept, Role> nonterminal_visitors);

    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;
};

/**
 * NonTerminal
 */

template<FeatureCategory D>
class RecurseNonTerminalVisitor : public NonTerminalVisitor<D>
{
public:
    void visit(NonTerminal<D> constructor) override;
};

/**
 * DerivationRule
 */

template<FeatureCategory D>
class RecurseDerivationRuleVisitor : public DerivationRuleVisitor<D>
{
protected:
    RecurseNonTerminalVisitor<D>& m_nonterminal_visitor;
    RecurseConstructorVisitor<D>& m_constructor_visitor;

public:
    explicit RecurseDerivationRuleVisitor(RecurseNonTerminalVisitor<D>& nonterminal_visitor, RecurseConstructorVisitor<D>& constructor_visitor);

    void visit(DerivationRule<D> rule) override;
};

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
class RecurseSubstitutionRuleVisitor : public SubstitutionRuleVisitor<D>
{
protected:
    RecurseNonTerminalVisitor<D>& m_nonterminal_visitor;

public:
    explicit RecurseSubstitutionRuleVisitor(RecurseNonTerminalVisitor<D>& nonterminal_visitor);

    void visit(SubstitutionRule<D> rule) override;
};

/**
 * Grammar
 */

class RecurseGrammarVisitor : public GrammarVisitor
{
protected:
    HanaRecurseNonTerminalVisitors<Concept, Role, Boolean, Numerical> m_start_symbol_visitor;

    HanaRecurseDerivationRuleVisitors<Concept, Role, Boolean, Numerical> m_derivation_rule_visitor;

    HanaRecurseSubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> m_substitution_rule_visitor;

public:
    RecurseGrammarVisitor(HanaRecurseNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                          HanaRecurseDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors,
                          HanaRecurseSubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> substitution_rule_visitors);

    void visit(const Grammar& grammar) override;
};

////////////////////////////
/// Copy
////////////////////////////

template<FeatureCategory D>
class CopyConstructorVisitor;
template<FeatureCategory... Ds>
using HanaCopyConstructorVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<CopyConstructorVisitor<Ds>>>...>;

template<FeatureCategory D>
class CopyNonTerminalVisitor;
template<FeatureCategory... Ds>
using HanaCopyNonTerminalVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<CopyNonTerminalVisitor<Ds>>>...>;

template<FeatureCategory D>
class CopyDerivationRuleVisitor;
template<FeatureCategory... Ds>
using HanaCopyDerivationRuleVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<CopyDerivationRuleVisitor<Ds>>>...>;

template<FeatureCategory D>
class CopySubstitutionRuleVisitor;
template<FeatureCategory... Ds>
using HanaCopySubstitutionRuleVisitors = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, std::reference_wrapper<CopySubstitutionRuleVisitor<Ds>>>...>;

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
    HanaCopyNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

    Constructor<Concept> m_result;  ///< the result of a visitation

public:
    CopyConstructorVisitor(ConstructorRepositories& repositories, HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors);

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
    HanaCopyNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

    Constructor<Role> m_result;  ///< the result of a visitation

public:
    CopyConstructorVisitor(ConstructorRepositories& repositories, HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors);

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
 * Booleans
 */

template<>
class CopyConstructorVisitor<Boolean> : public ConstructorVisitor<Boolean>
{
protected:
    ConstructorRepositories& m_repositories;
    HanaCopyNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

    Constructor<Boolean> m_result;

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories, HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors);

    void visit(BooleanAtomicState<Static> constructor) override;
    void visit(BooleanAtomicState<Fluent> constructor) override;
    void visit(BooleanAtomicState<Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;

    Constructor<Boolean> get_result() const;
};

/**
 * Numericals
 */

template<>
class CopyConstructorVisitor<Numerical> : public ConstructorVisitor<Numerical>
{
protected:
    ConstructorRepositories& m_repositories;
    HanaCopyNonTerminalVisitors<Concept, Role> m_nonterminal_visitors;

    Constructor<Numerical> m_result;

public:
    explicit CopyConstructorVisitor(ConstructorRepositories& repositories, HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors);

    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;

    Constructor<Numerical> get_result() const;
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

    HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> m_start_symbol_visitor;

    HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> m_derivation_rule_visitor;

    HanaCopySubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> m_substitution_rule_visitor;

public:
    CopyGrammarVisitor(ConstructorRepositories& repositories,
                       StartSymbolsContainer& start_symbols,
                       DerivationRulesContainer& derivation_rules,
                       SubstitutionRulesContainer& substitution_rules,
                       HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                       HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors,
                       HanaCopySubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> substitution_rule_visitors);

    void visit(const Grammar& grammar) override;
};

}

#endif
