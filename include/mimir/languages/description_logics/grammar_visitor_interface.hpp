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

#include <any>
#include <variant>

namespace mimir::languages::dl::grammar
{

////////////////////////////
/// Interface
////////////////////////////

class IVisitor
{
public:
    virtual ~IVisitor() = default;

    /* Concepts */
    virtual void visit(ConceptBot constructor) = 0;
    virtual void visit(ConceptTop constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::Static> constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::Derived> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::Static> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::Fluent> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::Derived> constructor) = 0;
    virtual void visit(ConceptNominal constructor) = 0;
    virtual void visit(ConceptIntersection constructor) = 0;
    virtual void visit(ConceptUnion constructor) = 0;
    virtual void visit(ConceptNegation constructor) = 0;
    virtual void visit(ConceptValueRestriction constructor) = 0;
    virtual void visit(ConceptExistentialQuantification constructor) = 0;
    virtual void visit(ConceptRoleValueMapContainment constructor) = 0;
    virtual void visit(ConceptRoleValueMapEquality constructor) = 0;
    /* Roles */
    virtual void visit(RoleUniversal constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::Static> constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::Fluent> constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::Derived> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::Static> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::Fluent> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::Derived> constructor) = 0;
    virtual void visit(RoleIntersection constructor) = 0;
    virtual void visit(RoleUnion constructor) = 0;
    virtual void visit(RoleComplement constructor) = 0;
    virtual void visit(RoleInverse constructor) = 0;
    virtual void visit(RoleComposition constructor) = 0;
    virtual void visit(RoleTransitiveClosure constructor) = 0;
    virtual void visit(RoleReflexiveTransitiveClosure constructor) = 0;
    virtual void visit(RoleRestriction constructor) = 0;
    virtual void visit(RoleIdentity constructor) = 0;
    /* Booleans */
    virtual void visit(BooleanAtomicState<formalism::Static> constructor) = 0;
    virtual void visit(BooleanAtomicState<formalism::Fluent> constructor) = 0;
    virtual void visit(BooleanAtomicState<formalism::Derived> constructor) = 0;
    virtual void visit(BooleanNonempty<Concept> constructor) = 0;
    virtual void visit(BooleanNonempty<Role> constructor) = 0;
    /* Numericals */
    virtual void visit(NumericalCount<Concept> constructor) = 0;
    virtual void visit(NumericalCount<Role> constructor) = 0;
    virtual void visit(NumericalDistance constructor) = 0;
    /* Nonterminals */
    virtual void visit(NonTerminal<Concept> constructor) = 0;
    virtual void visit(NonTerminal<Role> constructor) = 0;
    virtual void visit(NonTerminal<Boolean> constructor) = 0;
    virtual void visit(NonTerminal<Numerical> constructor) = 0;
    /* ConstructorOrNonterminals */
    virtual void visit(ConstructorOrNonTerminal<Concept> rule) = 0;
    virtual void visit(ConstructorOrNonTerminal<Role> rule) = 0;
    virtual void visit(ConstructorOrNonTerminal<Boolean> rule) = 0;
    virtual void visit(ConstructorOrNonTerminal<Numerical> rule) = 0;
    /* DerivationRules */
    virtual void visit(DerivationRule<Concept> rule) = 0;
    virtual void visit(DerivationRule<Role> rule) = 0;
    virtual void visit(DerivationRule<Boolean> rule) = 0;
    virtual void visit(DerivationRule<Numerical> rule) = 0;
    /* Grammar */
    virtual void visit(const Grammar& grammar) = 0;
};

////////////////////////////
/// Recurse
////////////////////////////

class RecurseVisitor : public IVisitor
{
public:
    /* Concepts */
    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<formalism::Static> constructor) override;
    void visit(ConceptAtomicState<formalism::Fluent> constructor) override;
    void visit(ConceptAtomicState<formalism::Derived> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Static> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Fluent> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Derived> constructor) override;
    void visit(ConceptNominal constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    /* Roles */
    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<formalism::Static> constructor) override;
    void visit(RoleAtomicState<formalism::Fluent> constructor) override;
    void visit(RoleAtomicState<formalism::Derived> constructor) override;
    void visit(RoleAtomicGoal<formalism::Static> constructor) override;
    void visit(RoleAtomicGoal<formalism::Fluent> constructor) override;
    void visit(RoleAtomicGoal<formalism::Derived> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;
    /* Booleans */
    void visit(BooleanAtomicState<formalism::Static> constructor) override;
    void visit(BooleanAtomicState<formalism::Fluent> constructor) override;
    void visit(BooleanAtomicState<formalism::Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;
    /* Numericals */
    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;
    /* Nonterminals */
    void visit(NonTerminal<Concept> constructor) override;
    void visit(NonTerminal<Role> constructor) override;
    void visit(NonTerminal<Boolean> constructor) override;
    void visit(NonTerminal<Numerical> constructor) override;
    /* ConstructorOrNonterminals */
    void visit(ConstructorOrNonTerminal<Concept> rule) override;
    void visit(ConstructorOrNonTerminal<Role> rule) override;
    void visit(ConstructorOrNonTerminal<Boolean> rule) override;
    void visit(ConstructorOrNonTerminal<Numerical> rule) override;
    /* DerivationRules */
    void visit(DerivationRule<Concept> rule) override;
    void visit(DerivationRule<Role> rule) override;
    void visit(DerivationRule<Boolean> rule) override;
    void visit(DerivationRule<Numerical> rule) override;
    /* Grammar */
    void visit(const Grammar& grammar) override;

private:
    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor);

    template<FeatureCategory D>
    void visit_impl(ConstructorOrNonTerminal<D> rule);

    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> rule);
};

////////////////////////////
/// Copy
////////////////////////////

class CopyVisitor : public IVisitor
{
protected:
    Repositories& m_repositories;
    OptionalNonTerminals& m_start_symbols;
    DerivationRuleSets& m_derivation_rules;

    std::any m_result;

public:
    CopyVisitor(Repositories& repositories, OptionalNonTerminals& start_symbols, DerivationRuleSets& derivation_rules);

    /* Concepts */
    void visit(ConceptBot constructor) override;
    void visit(ConceptTop constructor) override;
    void visit(ConceptAtomicState<formalism::Static> constructor) override;
    void visit(ConceptAtomicState<formalism::Fluent> constructor) override;
    void visit(ConceptAtomicState<formalism::Derived> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Static> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Fluent> constructor) override;
    void visit(ConceptAtomicGoal<formalism::Derived> constructor) override;
    void visit(ConceptNominal constructor) override;
    void visit(ConceptIntersection constructor) override;
    void visit(ConceptUnion constructor) override;
    void visit(ConceptNegation constructor) override;
    void visit(ConceptValueRestriction constructor) override;
    void visit(ConceptExistentialQuantification constructor) override;
    void visit(ConceptRoleValueMapContainment constructor) override;
    void visit(ConceptRoleValueMapEquality constructor) override;
    /* Roles */
    void visit(RoleUniversal constructor) override;
    void visit(RoleAtomicState<formalism::Static> constructor) override;
    void visit(RoleAtomicState<formalism::Fluent> constructor) override;
    void visit(RoleAtomicState<formalism::Derived> constructor) override;
    void visit(RoleAtomicGoal<formalism::Static> constructor) override;
    void visit(RoleAtomicGoal<formalism::Fluent> constructor) override;
    void visit(RoleAtomicGoal<formalism::Derived> constructor) override;
    void visit(RoleIntersection constructor) override;
    void visit(RoleUnion constructor) override;
    void visit(RoleComplement constructor) override;
    void visit(RoleInverse constructor) override;
    void visit(RoleComposition constructor) override;
    void visit(RoleTransitiveClosure constructor) override;
    void visit(RoleReflexiveTransitiveClosure constructor) override;
    void visit(RoleRestriction constructor) override;
    void visit(RoleIdentity constructor) override;
    /* Booleans */
    void visit(BooleanAtomicState<formalism::Static> constructor) override;
    void visit(BooleanAtomicState<formalism::Fluent> constructor) override;
    void visit(BooleanAtomicState<formalism::Derived> constructor) override;
    void visit(BooleanNonempty<Concept> constructor) override;
    void visit(BooleanNonempty<Role> constructor) override;
    /* Numericals */
    void visit(NumericalCount<Concept> constructor) override;
    void visit(NumericalCount<Role> constructor) override;
    void visit(NumericalDistance constructor) override;
    /* Nonterminals */
    void visit(NonTerminal<Concept> constructor) override;
    void visit(NonTerminal<Role> constructor) override;
    void visit(NonTerminal<Boolean> constructor) override;
    void visit(NonTerminal<Numerical> constructor) override;
    /* SubstitutionRules */
    void visit(ConstructorOrNonTerminal<Concept> rule) override;
    void visit(ConstructorOrNonTerminal<Role> rule) override;
    void visit(ConstructorOrNonTerminal<Boolean> rule) override;
    void visit(ConstructorOrNonTerminal<Numerical> rule) override;
    /* DerivationRules */
    void visit(DerivationRule<Concept> rule) override;
    void visit(DerivationRule<Role> rule) override;
    void visit(DerivationRule<Boolean> rule) override;
    void visit(DerivationRule<Numerical> rule) override;
    /* Grammar */
    void visit(const Grammar& grammar) override;

    const std::any& get_result() const;

private:
    template<FeatureCategory D>
    void visit_impl(ConstructorOrNonTerminal<D> rule);

    template<FeatureCategory D>
    void visit_impl(NonTerminal<D> constructor);

    template<FeatureCategory D>
    void visit_impl(DerivationRule<D> rule);
};

}

#endif
