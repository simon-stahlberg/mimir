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

using namespace mimir::formalism;

namespace mimir::languages::dl::grammar
{

////////////////////////////
/// Recursive Visitor
////////////////////////////

void RecurseVisitor::visit(ConceptBot constructor) {}
void RecurseVisitor::visit(ConceptTop constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<StaticTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<FluentTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor) {}
void RecurseVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    constructor->get_right_concept_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    constructor->get_right_concept_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptNegation constructor) { constructor->get_concept_or_non_terminal()->accept(*this); }
void RecurseVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    constructor->get_right_role_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    constructor->get_right_role_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(ConceptNominal constructor) {}

/**
 * Role
 */

void RecurseVisitor::visit(RoleUniversal constructor) {}
void RecurseVisitor::visit(RoleAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<StaticTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<FluentTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<DerivedTag> constructor) {}
void RecurseVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    constructor->get_right_role_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    constructor->get_right_role_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(RoleComplement constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void RecurseVisitor::visit(RoleInverse constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void RecurseVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    constructor->get_right_role_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(RoleTransitiveClosure constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void RecurseVisitor::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void RecurseVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void RecurseVisitor::visit(RoleIdentity constructor) { constructor->get_concept_or_non_terminal()->accept(*this); }

/**
 * Booleans
 */

void RecurseVisitor::visit(BooleanAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(BooleanNonempty<ConceptTag> constructor) { constructor->get_constructor_or_nonterminal()->accept(*this); }
void RecurseVisitor::visit(BooleanNonempty<RoleTag> constructor) { constructor->get_constructor_or_nonterminal()->accept(*this); }

/**
 * Numericals
 */

void RecurseVisitor::visit(NumericalCount<ConceptTag> constructor) { constructor->get_constructor_or_nonterminal()->accept(*this); }
void RecurseVisitor::visit(NumericalCount<RoleTag> constructor) { constructor->get_constructor_or_nonterminal()->accept(*this); }
void RecurseVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept_or_nonterminal()->accept(*this);
    constructor->get_role_or_nonterminal()->accept(*this);
    constructor->get_right_concept_or_nonterminal()->accept(*this);
}

/**
 * ConstructorOrRoleNonTerminal
 */

void RecurseVisitor::visit(ConstructorOrNonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(ConstructorOrNonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(ConstructorOrNonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(ConstructorOrNonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void RecurseVisitor::visit_impl(ConstructorOrNonTerminal<D> constructor)
{
    std::visit([this](auto&& arg) { arg->accept(*this); }, constructor->get_constructor_or_non_terminal());
}

template void RecurseVisitor::visit_impl(ConstructorOrNonTerminal<ConceptTag> constructor);
template void RecurseVisitor::visit_impl(ConstructorOrNonTerminal<RoleTag> constructor);
template void RecurseVisitor::visit_impl(ConstructorOrNonTerminal<BooleanTag> constructor);
template void RecurseVisitor::visit_impl(ConstructorOrNonTerminal<NumericalTag> constructor);

/**
 * NonTerminal
 */

void RecurseVisitor::visit(NonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(NonTerminal<RoleTag> constructor) {}

void RecurseVisitor::visit(NonTerminal<BooleanTag> constructor) {}

void RecurseVisitor::visit(NonTerminal<NumericalTag> constructor) {}

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void RecurseVisitor::visit_impl(NonTerminal<D> constructor)
{
}

template void RecurseVisitor::visit_impl(NonTerminal<ConceptTag> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<RoleTag> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<BooleanTag> constructor);
template void RecurseVisitor::visit_impl(NonTerminal<NumericalTag> constructor);

/**
 * DerivationRule
 */

void RecurseVisitor::visit(DerivationRule<ConceptTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<RoleTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<BooleanTag> constructor) { visit_impl(constructor); }

void RecurseVisitor::visit(DerivationRule<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void RecurseVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_non_terminal()->accept(*this);

    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*this);
    }
}

template void RecurseVisitor::visit_impl(DerivationRule<ConceptTag> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<RoleTag> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<BooleanTag> constructor);
template void RecurseVisitor::visit_impl(DerivationRule<NumericalTag> constructor);

/**
 * Grammar
 */

void RecurseVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);

                              if (second.has_value())
                              {
                                  second.value()->accept(*this);
                              }
                          });

    boost::hana::for_each(grammar.get_derivation_rules(),
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

CopyVisitor::CopyVisitor(Repositories& repositories, OptionalNonTerminals& start_symbols, DerivationRuleSets& derivation_rules) :
    m_repositories(repositories),
    m_start_symbols(start_symbols),
    m_derivation_rules(derivation_rules)
{
}

void CopyVisitor::visit(ConceptBot constructor) { m_result = m_repositories.get_or_create_concept_bot(); }
void CopyVisitor::visit(ConceptTop constructor) { m_result = m_repositories.get_or_create_concept_top(); }
void CopyVisitor::visit(ConceptAtomicState<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(ConceptAtomicState<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(ConceptAtomicState<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(ConceptAtomicGoal<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(ConceptAtomicGoal<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_concept_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_intersection(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept_or_non_terminal()->accept(*this);
    const auto left_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    constructor->get_right_concept_or_non_terminal()->accept(*this);
    const auto right_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_union(left_concept_or_nonterminal, right_concept_or_nonterminal);
}
void CopyVisitor::visit(ConceptNegation constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_negation(concept_or_nonterminal);
}
void CopyVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_value_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_existential_quantification(role_or_nonterminal, concept_or_nonterminal);
}
void CopyVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_role_value_map_containment(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_concept_role_value_map_equality(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyVisitor::visit(ConceptNominal constructor) { m_result = m_repositories.get_or_create_concept_nominal(constructor->get_object()); }

/**
 * Role
 */

void CopyVisitor::visit(RoleUniversal constructor) { m_result = m_repositories.get_or_create_role_universal(); }
void CopyVisitor::visit(RoleAtomicState<StaticTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicState<FluentTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicState<DerivedTag> constructor) { m_result = m_repositories.get_or_create_role_atomic_state(constructor->get_predicate()); }
void CopyVisitor::visit(RoleAtomicGoal<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(RoleAtomicGoal<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(RoleAtomicGoal<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_role_atomic_goal(constructor->get_predicate(), constructor->get_polarity());
}
void CopyVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_intersection(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_union(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyVisitor::visit(RoleComplement constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_complement(role_or_nonterminal);
}
void CopyVisitor::visit(RoleInverse constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_inverse(role_or_nonterminal);
}
void CopyVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role_or_non_terminal()->accept(*this);
    const auto left_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_role_or_non_terminal()->accept(*this);
    const auto right_role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_composition(left_role_or_nonterminal, right_role_or_nonterminal);
}
void CopyVisitor::visit(RoleTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_transitive_closure(role_or_nonterminal);
}
void CopyVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_role_reflexive_transitive_closure(role_or_nonterminal);
}
void CopyVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_role_restriction(role_or_nonterminal, concept_or_nonterminal);
}
void CopyVisitor::visit(RoleIdentity constructor)
{
    constructor->get_concept_or_non_terminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_role_identity(concept_or_nonterminal);
}

/**
 * Booleans
 */

void CopyVisitor::visit(BooleanAtomicState<StaticTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(BooleanAtomicState<FluentTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(BooleanAtomicState<DerivedTag> constructor)
{
    m_result = m_repositories.get_or_create_boolean_atomic_state(constructor->get_predicate());
}
void CopyVisitor::visit(BooleanNonempty<ConceptTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_boolean_nonempty(concept_or_nonterminal);
}
void CopyVisitor::visit(BooleanNonempty<RoleTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_boolean_nonempty(role_or_nonterminal);
}

/**
 * Numericals
 */

void CopyVisitor::visit(NumericalCount<ConceptTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_numerical_count(concept_or_nonterminal);
}
void CopyVisitor::visit(NumericalCount<RoleTag> constructor)
{
    constructor->get_constructor_or_nonterminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    m_result = m_repositories.get_or_create_numerical_count(role_or_nonterminal);
}
void CopyVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept_or_nonterminal()->accept(*this);
    const auto left_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    constructor->get_role_or_nonterminal()->accept(*this);
    const auto role_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<RoleTag>>(get_result());
    constructor->get_right_concept_or_nonterminal()->accept(*this);
    const auto right_concept_or_nonterminal = std::any_cast<ConstructorOrNonTerminal<ConceptTag>>(get_result());
    m_result = m_repositories.get_or_create_numerical_distance(left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal);
}

/**
 * CopyConstructorOrRoleNonTerminal
 */

void CopyVisitor::visit(ConstructorOrNonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(ConstructorOrNonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(ConstructorOrNonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(ConstructorOrNonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void CopyVisitor::visit_impl(ConstructorOrNonTerminal<D> constructor)
{
    std::visit(
        [this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            arg->accept(*this);

            if constexpr (std::is_same_v<T, Constructor<D>>)
            {
                m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(std::any_cast<Constructor<D>>(get_result()));
            }
            else if constexpr (std::is_same_v<T, NonTerminal<D>>)
            {
                m_result = this->m_repositories.template get_or_create_constructor_or_nonterminal<D>(std::any_cast<NonTerminal<D>>(get_result()));
            }
            else
            {
                static_assert(dependent_false<T>::value, "CopyVisitor::visit_impl(constructor): Undefined type for ConstructorOrNonTerminal.");
            }
        },
        constructor->get_constructor_or_non_terminal());
}

template void CopyVisitor::visit_impl(ConstructorOrNonTerminal<ConceptTag> constructor);
template void CopyVisitor::visit_impl(ConstructorOrNonTerminal<RoleTag> constructor);
template void CopyVisitor::visit_impl(ConstructorOrNonTerminal<BooleanTag> constructor);
template void CopyVisitor::visit_impl(ConstructorOrNonTerminal<NumericalTag> constructor);

/**
 * CopyNonTerminal
 */

void CopyVisitor::visit(NonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(NonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void CopyVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_result = m_repositories.template get_or_create_nonterminal<D>(constructor->get_name());
}

template void CopyVisitor::visit_impl(NonTerminal<ConceptTag> constructor);
template void CopyVisitor::visit_impl(NonTerminal<RoleTag> constructor);
template void CopyVisitor::visit_impl(NonTerminal<BooleanTag> constructor);
template void CopyVisitor::visit_impl(NonTerminal<NumericalTag> constructor);

/**
 * CopyDerivationRule
 */

void CopyVisitor::visit(DerivationRule<ConceptTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<RoleTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<BooleanTag> constructor) { visit_impl(constructor); }

void CopyVisitor::visit(DerivationRule<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void CopyVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_non_terminal()->accept(*this);
    const auto copied_non_terminal = std::any_cast<NonTerminal<D>>(get_result());

    auto copied_constructor_or_nonterminals = ConstructorOrNonTerminalList<D> {};
    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*this);
        copied_constructor_or_nonterminals.push_back(std::any_cast<ConstructorOrNonTerminal<D>>(get_result()));
    }

    m_result = m_repositories.template get_or_create_derivation_rule<D>(copied_non_terminal, std::move(copied_constructor_or_nonterminals));
}

template void CopyVisitor::visit_impl(DerivationRule<ConceptTag> constructor);
template void CopyVisitor::visit_impl(DerivationRule<RoleTag> constructor);
template void CopyVisitor::visit_impl(DerivationRule<BooleanTag> constructor);
template void CopyVisitor::visit_impl(DerivationRule<NumericalTag> constructor);

/**
 * Grammar
 */

void CopyVisitor::visit(const Grammar& grammar)
{
    boost::hana::for_each(grammar.get_start_symbols(),
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

    boost::hana::for_each(grammar.get_derivation_rules(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using FeatureType = typename decltype(+key)::type;

                              for (const auto& rule : second)
                              {
                                  rule->accept(*this);
                                  boost::hana::at_key(m_derivation_rules, key).insert(std::any_cast<DerivationRule<FeatureType>>(get_result()));
                              }
                          });
}

const std::any& CopyVisitor::get_result() const { return m_result; }
}
