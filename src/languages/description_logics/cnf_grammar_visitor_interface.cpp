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

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
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
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptNegation constructor) { constructor->get_concept()->accept(*this); }
void RecurseVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
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
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleComplement constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleInverse constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(RoleIdentity constructor) { constructor->get_concept()->accept(*this); }

/**
 * Booleans
 */

void RecurseVisitor::visit(BooleanAtomicState<Static> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(BooleanNonempty<Concept> constructor) { constructor->get_nonterminal()->accept(*this); }
void RecurseVisitor::visit(BooleanNonempty<Role> constructor) { constructor->get_nonterminal()->accept(*this); }

/**
 * Numericals
 */

void RecurseVisitor::visit(NumericalCount<Concept> constructor) { constructor->get_nonterminal()->accept(*this); }
void RecurseVisitor::visit(NumericalCount<Role> constructor) { constructor->get_nonterminal()->accept(*this); }
void RecurseVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_role()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}

/**
 * NonTerminal
 */

void RecurseVisitor::visit(NonTerminal<Concept> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(NonTerminal<Role> constructor) {}

void RecurseVisitor::visit(NonTerminal<Boolean> constructor) {}

void RecurseVisitor::visit(NonTerminal<Numerical> constructor) {}

template<FeatureCategory D>
void RecurseVisitor::visit_impl(NonTerminal<D> constructor)
{
}

template void RecurseVisitor::visit_impl(NonTerminal<Concept> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<Role> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<Boolean> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<Numerical> constructor);

/**
 * DerivationRule
 */

void RecurseVisitor::visit(DerivationRule<Concept> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<Role> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<Boolean> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void RecurseVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    constructor->get_body()->accept(*this);
}

template void RecurseVisitor::visit_impl(DerivationRule<Concept> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<Role> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<Boolean> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<Numerical> constructor);

/**
 * SubstitutionRule
 */

void RecurseVisitor::visit(SubstitutionRule<Concept> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(SubstitutionRule<Role> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(SubstitutionRule<Boolean> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(SubstitutionRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void RecurseVisitor::visit_impl(SubstitutionRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    constructor->get_body()->accept(*this);
}

template void RecurseVisitor::visit_impl(SubstitutionRule<Concept> constructor);
template void RecurseVisitor::visit_impl(SubstitutionRule<Role> constructor);
template void RecurseVisitor::visit_impl(SubstitutionRule<Boolean> constructor);
template void RecurseVisitor::visit_impl(SubstitutionRule<Numerical> constructor);

/**
 * Grammar
 */

void RecurseVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols_container(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);

                              if (second.has_value())
                              {
                                  second.value()->accept(*this);
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);

                              for (const auto& rule : second)
                              {
                                  rule->accept(*this);
                              }
                          });

    boost::hana::for_each(grammar.get_substitution_rules_container().get(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);

                              for (const auto& rule : second)
                              {
                                  rule->accept(*this);
                              }
                          });
}

////////////////////////////
/// Recursive Copy Visitor
////////////////////////////

CopyVisitor::CopyVisitor(Repositories& repositories,
                         OptionalNonTerminals& start_symbols,
                         DerivationRulesContainer& derivation_rules,
                         SubstitutionRulesContainer& substitution_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules),
    m_substitution_rules(substitution_rules)
{
}

void CopyVisitor::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void CopyVisitor::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void CopyVisitor::visit(ConceptAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(ConceptAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(ConceptAtomicState<Derived> constructor) { m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(ConceptAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept()->accept(*this);
    const auto left_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    constructor->get_right_concept()->accept(*this);
    const auto right_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_concept_intersection(left_concept, right_concept);
}
void CopyVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept()->accept(*this);
    const auto left_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    constructor->get_right_concept()->accept(*this);
    const auto right_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_concept_union(left_concept, right_concept);
}
void CopyVisitor::visit(ConceptNegation constructor)
{
    constructor->get_concept()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_concept_negation(concept_);
}
void CopyVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_concept()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_concept_value_restriction(role, concept_);
}
void CopyVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_concept()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_concept_existential_quantification(role, concept_);
}
void CopyVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role()->accept(*this);
    const auto left_role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_role()->accept(*this);
    const auto right_role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_role, right_role);
}
void CopyVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role()->accept(*this);
    const auto left_role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_role()->accept(*this);
    const auto right_role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_role, right_role);
}
void CopyVisitor::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

/**
 * Role
 */

void CopyVisitor::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void CopyVisitor::visit(RoleAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicState<Derived> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicGoal<Static> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(RoleAtomicGoal<Derived> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->is_negated());
}
void CopyVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role()->accept(*this);
    const auto left_role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_role()->accept(*this);
    const auto right_role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_intersection(left_role, right_role);
}
void CopyVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role()->accept(*this);
    const auto left_role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_role()->accept(*this);
    const auto right_role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_union(left_role, right_role);
}
void CopyVisitor::visit(RoleComplement constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_complement(role);
}
void CopyVisitor::visit(RoleInverse constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_inverse(role);
}
void CopyVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role()->accept(*this);
    const auto left_role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_role()->accept(*this);
    const auto right_role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_composition(left_role, right_role);
}
void CopyVisitor::visit(RoleTransitiveClosure constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_transitive_closure(role);
}
void CopyVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(role);
}
void CopyVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_concept()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_role_restriction(role, concept_);
}
void CopyVisitor::visit(RoleIdentity constructor)
{
    constructor->get_concept()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_role_identity(concept_);
}

/**
 * Booleans
 */

void CopyVisitor::visit(BooleanAtomicState<Static> constructor) { m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(BooleanAtomicState<Fluent> constructor) { m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(BooleanAtomicState<Derived> constructor) { m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(BooleanNonempty<Concept> constructor)
{
    constructor->get_nonterminal()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_);
}
void CopyVisitor::visit(BooleanNonempty<Role> constructor)
{
    constructor->get_nonterminal()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_boolean_nonempty(role);
}

/**
 * Numericals
 */

void CopyVisitor::visit(NumericalCount<Concept> constructor)
{
    constructor->get_nonterminal()->accept(*this);
    const auto concept_ = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_numerical_count(concept_);
}
void CopyVisitor::visit(NumericalCount<Role> constructor)
{
    constructor->get_nonterminal()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    m_result = m_repositories.get_or_create_numerical_count(role);
}
void CopyVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept()->accept(*this);
    const auto left_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    constructor->get_role()->accept(*this);
    const auto role = std::any_cast<NonTerminal<Role>>(get_result());
    constructor->get_right_concept()->accept(*this);
    const auto right_concept = std::any_cast<NonTerminal<Concept>>(get_result());
    m_result = m_repositories.get_or_create_numerical_distance(left_concept, role, right_concept);
}

/**
 * CopyNonTerminal
 */

void CopyVisitor::visit(NonTerminal<Concept> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<Role> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<Boolean> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void CopyVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template void CopyVisitor::visit_impl(NonTerminal<Concept> constructor);
template void CopyVisitor::visit_impl(NonTerminal<Role> constructor);
template void CopyVisitor::visit_impl(NonTerminal<Boolean> constructor);
template void CopyVisitor::visit_impl(NonTerminal<Numerical> constructor);

/**
 * CopyDerivationRule
 */

void CopyVisitor::visit(DerivationRule<Concept> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<Role> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<Boolean> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void CopyVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_head()->accept(*this);
    const auto head = std::any_cast<NonTerminal<D>>(get_result());

    constructor->get_body()->accept(*this);
    const auto body = std::any_cast<Constructor<D>>(get_result());

    m_result = m_repositories.template get_or_create_derivation_rule<D>(head, body);
}

template void CopyVisitor::visit_impl(DerivationRule<Concept> constructor);
template void CopyVisitor::visit_impl(DerivationRule<Role> constructor);
template void CopyVisitor::visit_impl(DerivationRule<Boolean> constructor);
template void CopyVisitor::visit_impl(DerivationRule<Numerical> constructor);

/**
 * CopySubstitutionRule
 */

void CopyVisitor::visit(SubstitutionRule<Concept> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(SubstitutionRule<Role> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(SubstitutionRule<Boolean> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(SubstitutionRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void CopyVisitor::visit_impl(SubstitutionRule<D> constructor)
{
    constructor->get_head()->accept(*this);
    const auto head = std::any_cast<NonTerminal<D>>(get_result());

    constructor->get_body()->accept(*this);
    const auto body = std::any_cast<NonTerminal<D>>(get_result());

    m_result = m_repositories.template get_or_create_substitution_rule<D>(head, body);
}

template void CopyVisitor::visit_impl(SubstitutionRule<Concept> constructor);
template void CopyVisitor::visit_impl(SubstitutionRule<Role> constructor);
template void CopyVisitor::visit_impl(SubstitutionRule<Boolean> constructor);
template void CopyVisitor::visit_impl(SubstitutionRule<Numerical> constructor);

/**
 * Grammar
 */

void CopyVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols_container(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  second.value()->accept(*this);
                                  boost::hana::at_key(m_start_symbols, key) = std::any_cast<NonTerminal<FeatureType>>(get_result());
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
                                  rule->accept(*this);
                                  m_derivation_rules.push_back(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                              }
                          });

    boost::hana::for_each(grammar.get_substitution_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  rule->accept(*this);
                                  m_substitution_rules.push_back(std::any_cast<SubstitutionRule<FeatureType>>(get_result()));
                              }
                          });
}

const std::any& CopyVisitor::get_result() const { return m_result; }
}
