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

#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/grammar_containers.hpp"

namespace mimir::dl::grammar
{

void ConstructorVisitor<Concept>::initialize(ConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                             ConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void ConstructorVisitor<Concept>::visit(ConceptBot constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptTop constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal_left()->accept(*m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal_right()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal_left()->accept(*m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal_right()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptNominal constructor) {}

/**
 * Role
 */

void ConstructorVisitor<Role>::initialize(ConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                          ConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void ConstructorVisitor<Role>::visit(RoleUniversal constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor) {}
void ConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void ConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
void ConstructorOrNonTerminalVisitor<D>::initialize(NonTerminalVisitor<D>& nonterminal_visitor, ConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<ConceptOrRole D>
void ConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                assert(m_constructor_visitor);
                arg->accept(*this->m_constructor_visitor);
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                assert(m_nonterminal_visitor);
                arg->accept(*this->m_nonterminal_visitor);
            }
            else
            {
                static_assert(dependent_false<D>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template class ConstructorOrNonTerminalVisitor<Concept>;
template class ConstructorOrNonTerminalVisitor<Role>;

/**
 * NonTerminal
 */

template<ConceptOrRole D>
void NonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
}

template class NonTerminalVisitor<Concept>;
template class NonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
void DerivationRuleVisitor<D>::initialize(NonTerminalVisitor<D>& nonterminal_visitor, ConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<ConceptOrRole D>
void DerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(m_nonterminal_visitor && m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);
    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*m_constructor_or_nonterminal_visitor);
    }
}

template class DerivationRuleVisitor<Concept>;
template class DerivationRuleVisitor<Role>;
}
