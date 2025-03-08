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

#include "mimir/languages/description_logics/grammar_visitor_interface.hpp"

#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/grammar_containers.hpp"

namespace mimir::dl::grammar
{

////////////////////////////
/// Recursive Visitor
////////////////////////////

void RecurseConstructorVisitor<Concept>::initialize(RecurseConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                                    RecurseConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void RecurseConstructorVisitor<Concept>::visit(ConceptBot constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptTop constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor) {}
void RecurseConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_left_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    constructor->get_right_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_left_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    constructor->get_right_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Concept>::visit(ConceptNominal constructor) {}

/**
 * Role
 */

void RecurseConstructorVisitor<Role>::initialize(RecurseConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                                 RecurseConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void RecurseConstructorVisitor<Role>::visit(RoleUniversal constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor) {}
void RecurseConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void RecurseConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
void RecurseConstructorOrNonTerminalVisitor<D>::initialize(RecurseNonTerminalVisitor<D>& nonterminal_visitor, RecurseConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<ConceptOrRole D>
void RecurseConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
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

template class RecurseConstructorOrNonTerminalVisitor<Concept>;
template class RecurseConstructorOrNonTerminalVisitor<Role>;

/**
 * NonTerminal
 */

template<ConceptOrRole D>
void RecurseNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
}

template class RecurseNonTerminalVisitor<Concept>;
template class RecurseNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
void RecurseDerivationRuleVisitor<D>::initialize(RecurseNonTerminalVisitor<D>& nonterminal_visitor,
                                                 RecurseConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<ConceptOrRole D>
void RecurseDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(m_nonterminal_visitor && m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);
    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*m_constructor_or_nonterminal_visitor);
    }
}

template class RecurseDerivationRuleVisitor<Concept>;
template class RecurseDerivationRuleVisitor<Role>;

/**
 * Grammar
 */

void RecurseGrammarVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              if (second.has_value())
                              {
                                  second.value()->accept(*boost::hana::at_key(m_start_symbol_visitor, key));
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& rule : second)
                              {
                                  rule->accept(*boost::hana::at_key(m_derivation_rule_visitor, key));
                              }
                          });
}

void RecurseGrammarVisitor::initialize(RecurseNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                                       RecurseNonTerminalVisitor<Role>& role_start_symbol_visitor,
                                       RecurseDerivationRuleVisitor<Concept>& concept_rule_visitor,
                                       RecurseDerivationRuleVisitor<Role>& role_rule_visitor)
{
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Concept> {}) = &concept_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Role> {}) = &role_start_symbol_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Concept> {}) = &concept_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Role> {}) = &role_rule_visitor;
}

////////////////////////////
/// Recursive Copy Visitor
////////////////////////////

CopyConstructorVisitor<Concept>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Concept>::initialize(CopyConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                                 CopyConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void CopyConstructorVisitor<Concept>::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void CopyConstructorVisitor<Concept>::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_left_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto left_concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto right_concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_left_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto left_concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto right_concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_union(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_negation(concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_value_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_existential_quantification(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

Constructor<Concept> CopyConstructorVisitor<Concept>::get_result() const { return m_result; }

/**
 * Role
 */

CopyConstructorVisitor<Role>::CopyConstructorVisitor(ConstructorRepositories& repositories) : m_repositories(repositories) {}

void CopyConstructorVisitor<Role>::initialize(CopyConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                              CopyConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

void CopyConstructorVisitor<Role>::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_intersection(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_union(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_complement(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_inverse(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_left_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_composition(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_transitive_closure(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    const auto role_or_nonterminal = m_role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = m_concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_identity(concept_or_nonterminal);
}

Constructor<Role> CopyConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * CopyConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
CopyConstructorOrNonTerminalVisitor<D>::CopyConstructorOrNonTerminalVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyConstructorOrNonTerminalVisitor<D>::initialize(CopyNonTerminalVisitor<D>& nonterminal_visitor, CopyConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<ConceptOrRole D>
void CopyConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                assert(this->m_constructor_visitor);
                arg->accept(*this->m_constructor_visitor);
                m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(this->m_constructor_visitor->get_result());
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                assert(this->m_nonterminal_visitor);
                arg->accept(*this->m_nonterminal_visitor);
                m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(this->m_nonterminal_visitor->get_result());
            }
            else
            {
                static_assert(dependent_false<D>::value,
                              "ConstructorOrNonTerminalVisitor<D>::visit(constructor): Missing implementation for ConstructorOrNonTerminal type.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template<ConceptOrRole D>
ConstructorOrNonTerminal<D> CopyConstructorOrNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyConstructorOrNonTerminalVisitor<Concept>;
template class CopyConstructorOrNonTerminalVisitor<Role>;

/**
 * CopyNonTerminal
 */

template<ConceptOrRole D>
CopyNonTerminalVisitor<D>::CopyNonTerminalVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template<ConceptOrRole D>
NonTerminal<D> CopyNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyNonTerminalVisitor<Concept>;
template class CopyNonTerminalVisitor<Role>;

/**
 * CopyDerivationRule
 */

template<ConceptOrRole D>
CopyDerivationRuleVisitor<D>::CopyDerivationRuleVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<ConceptOrRole D>
void CopyDerivationRuleVisitor<D>::initialize(CopyNonTerminalVisitor<D>& nonterminal_visitor,
                                              CopyConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<ConceptOrRole D>
void CopyDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(m_nonterminal_visitor && m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);
    const auto copied_non_terminal = m_nonterminal_visitor->get_result();

    auto copied_constructor_or_nonterminals = ConstructorOrNonTerminalList<D> {};
    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*m_constructor_or_nonterminal_visitor);
        copied_constructor_or_nonterminals.push_back(m_constructor_or_nonterminal_visitor->get_result());
    }

    m_result = m_repositories.template get_or_create_derivation_rule<D>(copied_non_terminal, std::move(copied_constructor_or_nonterminals));
}

template<ConceptOrRole D>
DerivationRule<D> CopyDerivationRuleVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyDerivationRuleVisitor<Concept>;
template class CopyDerivationRuleVisitor<Role>;

/**
 * Grammar
 */

CopyGrammarVisitor::CopyGrammarVisitor(ConstructorRepositories& repositories,
                                       StartSymbolsContainer& start_symbols,
                                       DerivationRulesContainer& derivation_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules)
{
}

void CopyGrammarVisitor::visit(const Grammar& grammar)
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
                                  const auto copied_rule = visitor.get_result();
                                  m_derivation_rules.insert(copied_rule);
                              }
                          });
}

void CopyGrammarVisitor::initialize(CopyNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                                    CopyNonTerminalVisitor<Role>& role_start_symbol_visitor,
                                    CopyDerivationRuleVisitor<Concept>& concept_rule_visitor,
                                    CopyDerivationRuleVisitor<Role>& role_rule_visitor)
{
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Concept> {}) = &concept_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Role> {}) = &role_start_symbol_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Concept> {}) = &concept_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Role> {}) = &role_rule_visitor;
}

}
