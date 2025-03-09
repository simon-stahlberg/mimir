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

#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructor_repositories.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"

namespace mimir::dl::cnf_grammar
{

CopyConstructorVisitor<Concept>::CopyConstructorVisitor(ConstructorRepositories& repositories,
                                                        HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors) :
    m_repositories(repositories),
    m_nonterminal_visitors(nonterminal_visitors)
{
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
void CopyConstructorVisitor<Concept>::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }
void CopyConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_left_concept()->accept(concept_visitor);
    const auto left_nonterminal = concept_visitor.get_result();
    constructor->get_right_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_intersection(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_left_concept()->accept(concept_visitor);
    const auto left_nonterminal = concept_visitor.get_result();
    constructor->get_right_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_union(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_concept()->accept(concept_visitor);
    const auto nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_negation(nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_value_restriction(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_existential_quantification(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_left_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_right_role()->accept(role_visitor);
    const auto right_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    auto visitor = CopyNonTerminalVisitor<Role>(m_repositories);
    constructor->get_left_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_right_role()->accept(role_visitor);
    const auto right_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_nonterminal, right_nonterminal);
}

const Constructor<Concept>& CopyConstructorVisitor<Concept>::get_result() const { return m_result; }

/**
 * Role
 */

CopyConstructorVisitor<Role>::CopyConstructorVisitor(ConstructorRepositories& repositories, HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors) :
    m_repositories(repositories),
    m_nonterminal_visitors(nonterminal_visitors)
{
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
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_left_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_right_role()->accept(role_visitor);
    const auto right_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_intersection(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_left_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_right_role()->accept(role_visitor);
    const auto right_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_union(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_complement(nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_inverse(nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_left_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_right_role()->accept(role_visitor);
    const auto right_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_composition(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_transitive_closure(nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_role()->accept(role_visitor);
    const auto left_nonterminal = role_visitor.get_result();
    constructor->get_concept()->accept(concept_visitor);
    const auto right_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_role_restriction(left_nonterminal, right_nonterminal);
}
void CopyConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_concept()->accept(concept_visitor);
    const auto nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_role_identity(nonterminal);
}

const Constructor<Role>& CopyConstructorVisitor<Role>::get_result() const { return m_result; }

/**
 * Booleans
 */

CopyConstructorVisitor<Boolean>::CopyConstructorVisitor(ConstructorRepositories& repositories,
                                                        HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors) :
    m_repositories(repositories),
    m_nonterminal_visitors(nonterminal_visitors)
{
}

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
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_nonterminal()->accept(concept_visitor);
    const auto concept_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_nonterminal);
}

void CopyConstructorVisitor<Boolean>::visit(BooleanNonempty<Role> constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_nonterminal()->accept(role_visitor);
    const auto role_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_boolean_nonempty(role_nonterminal);
}

Constructor<Boolean> CopyConstructorVisitor<Boolean>::get_result() const { return m_result; }

/**
 * Numericals
 */

CopyConstructorVisitor<Numerical>::CopyConstructorVisitor(ConstructorRepositories& repositories,
                                                          HanaCopyNonTerminalVisitors<Concept, Role> nonterminal_visitors) :
    m_repositories(repositories),
    m_nonterminal_visitors(nonterminal_visitors)
{
}

void CopyConstructorVisitor<Numerical>::visit(NumericalCount<Concept> constructor)
{
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_nonterminal()->accept(concept_visitor);
    const auto concept_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_numerical_count(concept_nonterminal);
}

void CopyConstructorVisitor<Numerical>::visit(NumericalCount<Role> constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    constructor->get_nonterminal()->accept(role_visitor);
    const auto role_nonterminal = role_visitor.get_result();
    m_result = m_repositories.get_or_create_numerical_count(role_nonterminal);
}

void CopyConstructorVisitor<Numerical>::visit(NumericalDistance constructor)
{
    auto& role_visitor = get_visitor<Role>(m_nonterminal_visitors);
    auto& concept_visitor = get_visitor<Concept>(m_nonterminal_visitors);
    constructor->get_left_concept()->accept(concept_visitor);
    const auto left_concept_nonterminal = concept_visitor.get_result();
    constructor->get_role()->accept(role_visitor);
    const auto role_nonterminal = role_visitor.get_result();
    constructor->get_right_concept()->accept(concept_visitor);
    const auto right_concept_nonterminal = concept_visitor.get_result();
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_nonterminal, role_nonterminal, right_concept_nonterminal);
}

Constructor<Numerical> CopyConstructorVisitor<Numerical>::get_result() const { return m_result; }

/**
 * NonTerminal
 */

template<FeatureCategory D>
CopyNonTerminalVisitor<D>::CopyNonTerminalVisitor(ConstructorRepositories& repositories) : m_repositories(repositories)
{
}

template<FeatureCategory D>
void CopyNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_result = m_repositories.get_or_create_nonterminal<D>(constructor->get_name());
}

template<FeatureCategory D>
const NonTerminal<D>& CopyNonTerminalVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyNonTerminalVisitor<Concept>;
template class CopyNonTerminalVisitor<Role>;
template class CopyNonTerminalVisitor<Boolean>;
template class CopyNonTerminalVisitor<Numerical>;

/**
 * DerivationRule
 */

template<FeatureCategory D>
CopyDerivationRuleVisitor<D>::CopyDerivationRuleVisitor(ConstructorRepositories& repositories,
                                                        CopyNonTerminalVisitor<D>& nonterminal_visitor,
                                                        CopyConstructorVisitor<D>& constructor_visitor) :
    m_repositories(repositories),
    m_nonterminal_visitor(nonterminal_visitor),
    m_constructor_visitor(constructor_visitor)
{
}

template<FeatureCategory D>
void CopyDerivationRuleVisitor<D>::visit(DerivationRule<D> rule)
{
    rule->get_head()->accept(m_nonterminal_visitor);
    const auto head = m_nonterminal_visitor.get_result();
    rule->get_body()->accept(m_constructor_visitor);
    const auto body = m_constructor_visitor.get_result();
    m_result = m_repositories.get_or_create_derivation_rule(head, body);
}

template<FeatureCategory D>
const DerivationRule<D>& CopyDerivationRuleVisitor<D>::get_result() const
{
    return m_result;
}

template class CopyDerivationRuleVisitor<Concept>;
template class CopyDerivationRuleVisitor<Role>;
template class CopyDerivationRuleVisitor<Boolean>;
template class CopyDerivationRuleVisitor<Numerical>;

/**
 * SubstitutionRule
 */

template<FeatureCategory D>
CopySubstitutionRuleVisitor<D>::CopySubstitutionRuleVisitor(ConstructorRepositories& repositories, CopyNonTerminalVisitor<D>& nonterminal_visitor) :
    m_repositories(repositories),
    m_nonterminal_visitor(nonterminal_visitor)
{
}

template<FeatureCategory D>
void CopySubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> rule)
{
    rule->get_head()->accept(m_nonterminal_visitor);
    const auto head = m_nonterminal_visitor.get_result();
    rule->get_body()->accept(m_nonterminal_visitor);
    const auto body = m_nonterminal_visitor.get_result();
    m_result = m_repositories.get_or_create_substitution_rule(head, body);
}

template<FeatureCategory D>
const SubstitutionRule<D>& CopySubstitutionRuleVisitor<D>::get_result() const
{
    return m_result;
}

template class CopySubstitutionRuleVisitor<Concept>;
template class CopySubstitutionRuleVisitor<Role>;
template class CopySubstitutionRuleVisitor<Boolean>;
template class CopySubstitutionRuleVisitor<Numerical>;

/**
 * Grammar
 */

CopyGrammarVisitor::CopyGrammarVisitor(ConstructorRepositories& repositories,
                                       StartSymbolsContainer& start_symbols,
                                       DerivationRulesContainer& derivation_rules,
                                       SubstitutionRulesContainer& substitution_rules,
                                       HanaCopyNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                                       HanaCopyDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors,
                                       HanaCopySubstitutionRuleVisitors<Concept, Role, Boolean, Numerical> substitution_rule_visitors) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules),
    m_substitution_rules(substitution_rules),
    m_start_symbol_visitor(std::move(start_symbol_visitors)),
    m_derivation_rule_visitor(std::move(derivation_rule_visitors)),
    m_substitution_rule_visitor(std::move(substitution_rule_visitors))
{
}

void CopyGrammarVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using ConstructorType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  auto& visitor = get_visitor<ConstructorType>(m_start_symbol_visitor);
                                  second.value()->accept(visitor);
                                  m_start_symbols.insert(visitor.get_result());
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using ConstructorType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  auto& visitor = get_visitor<ConstructorType>(m_derivation_rule_visitor);
                                  rule->accept(visitor);
                                  const auto copied_rule = visitor.get_result();
                                  m_derivation_rules.push_back(copied_rule);
                              }
                          });

    boost::hana::for_each(grammar.get_substitution_rules().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using ConstructorType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  auto& visitor = get_visitor<ConstructorType>(m_substitution_rule_visitor);
                                  rule->accept(visitor);
                                  const auto copied_rule = visitor.get_result();
                                  m_substitution_rules.push_back(copied_rule);
                              }
                          });
}

}
