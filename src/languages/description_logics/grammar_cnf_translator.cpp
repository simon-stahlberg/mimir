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

#include "mimir/languages/description_logics/grammar_cnf_translator.hpp"

#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

namespace mimir::dl::grammar
{

/**
 * A ::= B | C  translate to
 *
 * A ::= B
 * A ::= C
 */
class FlattenChoicesVisitor
{
private:
    // Store the return value of a recursive call.
    NonTerminal<Concept> m_substituted_concept;
    NonTerminal<Role> m_substituted_role;

public:
    /* Concepts */
    virtual void visit(DerivationRule<Concept> constructor);
    virtual void visit(NonTerminal<Concept> constructor);
    virtual void visit(ConstructorOrNonTerminal<Concept> constructor);
    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<Static> constructor);
    virtual void visit(ConceptAtomicState<Fluent> constructor);
    virtual void visit(ConceptAtomicState<Derived> constructor);
    virtual void visit(ConceptAtomicGoal<Static> constructor);
    virtual void visit(ConceptAtomicGoal<Fluent> constructor);
    virtual void visit(ConceptAtomicGoal<Derived> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);

    /* Roles */
    virtual void visit(DerivationRule<Role> constructor);
    virtual void visit(NonTerminal<Role> constructor);
    virtual void visit(ConstructorOrNonTerminal<Role> constructor);
    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<Static> constructor);
    virtual void visit(RoleAtomicState<Fluent> constructor);
    virtual void visit(RoleAtomicState<Derived> constructor);
    virtual void visit(RoleAtomicGoal<Static> constructor);
    virtual void visit(RoleAtomicGoal<Fluent> constructor);
    virtual void visit(RoleAtomicGoal<Derived> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * A ::= B a C b translates to
 *
 * A ::= B D C E
 * D ::= a
 * E ::= b
 */
class SubstitutePrimitivesHelperVisitor
{
private:
    // Store the return value of a recursive call.
    NonTerminal<Concept> m_substituted_concept;
    NonTerminal<Role> m_substituted_role;

public:
    /* Concepts */
    virtual void visit(DerivationRule<Concept> constructor);
    virtual void visit(NonTerminal<Concept> constructor);
    virtual void visit(ConstructorOrNonTerminal<Concept> constructor);
    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<Static> constructor);
    virtual void visit(ConceptAtomicState<Fluent> constructor);
    virtual void visit(ConceptAtomicState<Derived> constructor);
    virtual void visit(ConceptAtomicGoal<Static> constructor);
    virtual void visit(ConceptAtomicGoal<Fluent> constructor);
    virtual void visit(ConceptAtomicGoal<Derived> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);

    /* Roles */
    virtual void visit(DerivationRule<Role> constructor);
    virtual void visit(NonTerminal<Role> constructor);
    virtual void visit(ConstructorOrNonTerminal<Role> constructor);
    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<Static> constructor);
    virtual void visit(RoleAtomicState<Fluent> constructor);
    virtual void visit(RoleAtomicState<Derived> constructor);
    virtual void visit(RoleAtomicGoal<Static> constructor);
    virtual void visit(RoleAtomicGoal<Fluent> constructor);
    virtual void visit(RoleAtomicGoal<Derived> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * On the first level in a body, we traverse only into the `SubstitutePrimitivesHelperVisitor`
 * to avoid substituting primitives in rules that only contain a single primitive.
 */
class SubstitutePrimitivesVisitor : public Visitor
{
public:
    /* Concepts */
    virtual void visit(DerivationRule<Concept> constructor);
    virtual void visit(NonTerminal<Concept> constructor);
    virtual void visit(ConstructorOrNonTerminal<Concept> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);

    /* Roles */
    virtual void visit(DerivationRule<Role> constructor);
    virtual void visit(NonTerminal<Role> constructor);
    virtual void visit(ConstructorOrNonTerminal<Role> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * A ::= B D C E translates to
 *
 * A ::= B D F
 * F ::= C E
 */
class SubstituteCompositeHelperVisitor
{
private:
    // Store the return value of a recursive call.
    Constructor<Concept> m_substituted_concept;
    Constructor<Role> m_substituted_role;

public:
    /* Concepts */
    virtual void visit(DerivationRule<Concept> constructor);
    virtual void visit(NonTerminal<Concept> constructor);
    virtual void visit(ConstructorOrNonTerminal<Concept> constructor);
    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<Static> constructor);
    virtual void visit(ConceptAtomicState<Fluent> constructor);
    virtual void visit(ConceptAtomicState<Derived> constructor);
    virtual void visit(ConceptAtomicGoal<Static> constructor);
    virtual void visit(ConceptAtomicGoal<Fluent> constructor);
    virtual void visit(ConceptAtomicGoal<Derived> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);

    /* Roles */
    virtual void visit(DerivationRule<Role> constructor);
    virtual void visit(NonTerminal<Role> constructor);
    virtual void visit(ConstructorOrNonTerminal<Role> constructor);
    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<Static> constructor);
    virtual void visit(RoleAtomicState<Fluent> constructor);
    virtual void visit(RoleAtomicState<Derived> constructor);
    virtual void visit(RoleAtomicGoal<Static> constructor);
    virtual void visit(RoleAtomicGoal<Fluent> constructor);
    virtual void visit(RoleAtomicGoal<Derived> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * On the first level in a body, we traverse only into the `SubstituteCompositesHelperVisitor`
 * to avoid substituting composites in rules that only contain a single composite.
 */
class SubstituteCompositesVisitor : public Visitor
{
public:
    /* Concepts */
    virtual void visit(DerivationRule<Concept> constructor);
    virtual void visit(NonTerminal<Concept> constructor);
    virtual void visit(ConstructorOrNonTerminal<Concept> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);

    /* Roles */
    virtual void visit(DerivationRule<Role> constructor);
    virtual void visit(NonTerminal<Role> constructor);
    virtual void visit(ConstructorOrNonTerminal<Role> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

Grammar translate_to_cnf(const Grammar& grammar)
{
    auto start_symbols = StartSymbols();
    auto grammar_rules = GrammarRules {};
    auto repositories = GrammarConstructorRepositories();

    /* Step 1: flatten A ::= B | C  ==> A ::= B and A::= C */

    boost::hana::for_each(grammar.get_rules(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& non_terminal_and_rules : second)
                              {
                                  const auto& [non_terminal, rules] = non_terminal_and_rules;

                                  for (const auto& rule : rules)
                                  {
                                      for (const auto& choice : rule->get_constructor_or_non_terminals())
                                      {
                                          //  boost::hana::at_key(grammar_rules, key).
                                      }
                                  }
                              }
                          });
    {
    }

    auto visitor1 = FlattenChoicesVisitor();

    auto visitor2 = SubstitutePrimitivesVisitor();

    auto visitor3 = SubstituteCompositesVisitor();
}
}
