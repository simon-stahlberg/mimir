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

#include "mimir/languages/description_logics/cnf_grammar_visitor_interface.hpp"

#include "mimir/languages/description_logics/cnf_grammar_constructor_repositories.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"

namespace mimir::dl::cnf_grammar
{

CopyConstructorVisitor<Concept>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Concept>::visit(ConceptBot constructor) { m_repositories.get_or_create_concept_bot(); }
void CopyConstructorVisitor<Concept>::visit(ConceptTop constructor) { m_repositories.get_or_create_concept_top(); }
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_repositories.get_or_create_concept_nominal(constructor->get_object()); }
void CopyConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    auto visitor = CopyNonTerminalVisitor<Concept>(m_repositories);
    constructor->get_concept_left()->accept(visitor);
    const auto left_nonterminal = visitor.get_result();
    constructor->get_concept_right()->accept(visitor);
    const auto right_nonterminal = visitor.get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    auto visitor = CopyNonTerminalVisitor<Concept>(m_repositories);
    constructor->get_concept_left()->accept(visitor);
    const auto left_nonterminal = visitor.get_result();
    constructor->get_concept_right()->accept(visitor);
    const auto right_nonterminal = visitor.get_result();
    m_result = m_repositories.get_or_create_concept_union(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    auto visitor = CopyNonTerminalVisitor<Concept>(m_repositories);
    constructor->get_concept()->accept(visitor);
    const auto nonterminal = visitor.get_result();
    m_result = m_repositories.get_or_create_concept_negation(nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    auto concept_visitor = CopyNonTerminalVisitor<Concept>(m_repositories);
    auto role_visitor = CopyNonTerminalVisitor<Role>(m_repositories);
    constructor->get_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_value_restriction(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    auto concept_visitor = CopyNonTerminalVisitor<Concept>(m_repositories);
    auto role_visitor = CopyNonTerminalVisitor<Role>(m_repositories);
    constructor->get_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_existential_quantification(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    auto visitor = CopyNonTerminalVisitor<Role>(m_repositories);
    constructor->get_role_left()->accept(visitor);
    const auto left_nonterminal = visitor.get_result();
    constructor->get_role_right()->accept(visitor);
    const auto right_nonterminal = visitor.get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    auto visitor = CopyNonTerminalVisitor<Role>(m_repositories);
    constructor->get_role_left()->accept(visitor);
    const auto left_nonterminal = visitor.get_result();
    constructor->get_role_right()->accept(visitor);
    const auto right_nonterminal = visitor.get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}

const Constructor<Concept>& CopyConstructorVisitor<Concept>::get_result() const { return m_result; }

/**
 * Role
 */

CopyConstructorVisitor<Role>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Role>::visit(RoleUniversal constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleIntersection constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleUnion constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleComplement constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleInverse constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleComposition constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleRestriction constructor) {}
void CopyConstructorVisitor<Role>::visit(RoleIdentity constructor) {}

const Constructor<Role>& CopyConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * NonTerminal
 */

template<ConceptOrRole D>
CopyNonTerminalVisitor<D>::CopyNonTerminalVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
}

template<ConceptOrRole D>
const NonTerminal<D>& CopyNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyNonTerminalVisitor<Concept>;
template class CopyNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
CopyDerivationRuleVisitor<D>::CopyDerivationRuleVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyDerivationRuleVisitor<D>::visit(DerivationRule<D> rule)
{
}

template<ConceptOrRole D>
const DerivationRule<D>& CopyDerivationRuleVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyDerivationRuleVisitor<Concept>;
template class CopyDerivationRuleVisitor<Role>;

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
CopySubstitutionRuleVisitor<D>::CopySubstitutionRuleVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopySubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> rule)
{
}

template<ConceptOrRole D>
const SubstitutionRule<D>& CopySubstitutionRuleVisitor<D>::get_result() const
{
    return m_result;
}

template class CopySubstitutionRuleVisitor<Concept>;
template class CopySubstitutionRuleVisitor<Role>;

/**
 * Grammar
 */

CopyGrammarVisitor::CopyGrammarVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyGrammarVisitor::visit(const Grammar& grammar) {}

}
