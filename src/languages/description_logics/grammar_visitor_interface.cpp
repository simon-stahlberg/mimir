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

void RecurseVisitor::visit(ConceptBot constructor) {}
void RecurseVisitor::visit(ConceptTop constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Static> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Static> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Fluent> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Derived> constructor) {}
void RecurseVisitor::visit(ConceptIntersection constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptUnion constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptNegation constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptValueRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptExistentialQuantification constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(ConceptNominal constructor) {}

/**
 * Role
 */

void RecurseVisitor::visit(RoleUniversal constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Static> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Static> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Fluent> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Derived> constructor) {}
void RecurseVisitor::visit(RoleIntersection constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleUnion constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleComplement constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleInverse constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleComposition constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(RoleIdentity constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}

/**
 * Booleans
 */

void RecurseVisitor::visit(BooleanAtomicState<Static> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(BooleanNonempty<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(BooleanNonempty<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
}

/**
 * Numericals
 */

void RecurseVisitor::visit(NumericalCount<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}
void RecurseVisitor::visit(NumericalCount<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
}
void RecurseVisitor::visit(NumericalDistance constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_left_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    constructor->get_role_or_nonterminal()->accept(*role_or_nonterminal_visitor);
    constructor->get_right_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}

/**
 * ConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
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
template class RecurseConstructorOrNonTerminalVisitor<Boolean>;
template class RecurseConstructorOrNonTerminalVisitor<Numerical>;

/**
 * NonTerminal
 */

template<FeatureCategory D>
void RecurseNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
}

template class RecurseNonTerminalVisitor<Concept>;
template class RecurseNonTerminalVisitor<Role>;
template class RecurseNonTerminalVisitor<Boolean>;
template class RecurseNonTerminalVisitor<Numerical>;

/**
 * DerivationRule
 */

template<FeatureCategory D>
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
template class RecurseDerivationRuleVisitor<Boolean>;
template class RecurseDerivationRuleVisitor<Numerical>;

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
                              using FeatureType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  auto visitor = get_visitor<FeatureType>(m_start_symbol_visitors);
                                  assert(visitor);

                                  second.value()->accept(*visitor);
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  auto visitor = get_visitor<FeatureType>(m_derivation_rule_visitors);
                                  assert(visitor);

                                  rule->accept(*visitor);
                              }
                          });
}

////////////////////////////
/// Recursive Copy Visitor
////////////////////////////

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
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto left_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto right_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto left_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto right_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_union(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_negation(concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_value_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_existential_quantification(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

/**
 * Role
 */

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
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_intersection(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_union(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_complement(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_inverse(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto left_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto right_role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_composition(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_transitive_closure(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(role_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_role_identity(concept_or_nonterminal);
}

Constructor<Role> CopyConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * Booleans
 */

void CopyConstructorVisitor<Boolean>::visit(BooleanAtomicState<Static> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Boolean>::visit(BooleanAtomicState<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Boolean>::visit(BooleanAtomicState<Derived> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyConstructorVisitor<Boolean>::visit(BooleanNonempty<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_or_nonterminal);
}
void CopyConstructorVisitor<Boolean>::visit(BooleanNonempty<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(role_or_nonterminal);
}

Constructor<Boolean> CopyConstructorVisitor<Boolean>::get_result() const { return m_result; }

/**
 * Numericals
 */

void CopyConstructorVisitor<Numerical>::visit(NumericalCount<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    const auto concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_count(concept_or_nonterminal);
}
void CopyConstructorVisitor<Numerical>::visit(NumericalCount<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(role_or_nonterminal_visitor);

    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_count(role_or_nonterminal);
}
void CopyConstructorVisitor<Numerical>::visit(NumericalDistance constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_constructor_or_nonterminal_visitor);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_constructor_or_nonterminal_visitor);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    constructor->get_left_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    const auto left_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    constructor->get_role_or_nonterminal()->accept(*role_or_nonterminal_visitor);
    const auto role_or_nonterminal = role_or_nonterminal_visitor->get_result();
    constructor->get_right_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    const auto right_concept_or_nonterminal = concept_or_nonterminal_visitor->get_result();
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal);
}

/**
 * CopyConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
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

template class CopyConstructorOrNonTerminalVisitor<Concept>;
template class CopyConstructorOrNonTerminalVisitor<Role>;
template class CopyConstructorOrNonTerminalVisitor<Boolean>;
template class CopyConstructorOrNonTerminalVisitor<Numerical>;

/**
 * CopyNonTerminal
 */

template<FeatureCategory D>
void CopyNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template class CopyNonTerminalVisitor<Concept>;
template class CopyNonTerminalVisitor<Role>;
template class CopyNonTerminalVisitor<Boolean>;
template class CopyNonTerminalVisitor<Numerical>;

/**
 * CopyDerivationRule
 */

template<FeatureCategory D>
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

template class CopyDerivationRuleVisitor<Concept>;
template class CopyDerivationRuleVisitor<Role>;
template class CopyDerivationRuleVisitor<Boolean>;
template class CopyDerivationRuleVisitor<Numerical>;

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
                              using FeatureType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  auto visitor = get_visitor<FeatureType>(m_start_symbol_visitors);
                                  assert(visitor);

                                  second.value()->accept(*visitor);
                                  m_start_symbols.insert(visitor->get_result());
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  auto visitor = get_visitor<FeatureType>(m_derivation_rule_visitors);
                                  assert(visitor);

                                  rule->accept(*visitor);
                                  const auto copied_rule = visitor->get_result();
                                  m_derivation_rules.insert(copied_rule);
                              }
                          });
}

void CopyGrammarVisitor::initialize(HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                                    HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors)
{
    m_start_symbol_visitors = start_symbol_visitors;
    m_derivation_rule_visitors = derivation_rule_visitors;
}
}
