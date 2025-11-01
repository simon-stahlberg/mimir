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

#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"

#include <iostream>
#include <variant>

using namespace mimir::formalism;

namespace mimir::languages::dl::cnf_grammar
{

FormatterVisitor::FormatterVisitor(std::ostream& out) : m_out(out) {}

void FormatterVisitor::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterVisitor::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterVisitor::visit(ConceptAtomicState<StaticTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<FluentTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<DerivedTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicGoal<StaticTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptAtomicGoal<FluentTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(ConceptNominal constructor) { m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name(); }

/**
 * Role
 */

void FormatterVisitor::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterVisitor::visit(RoleAtomicState<StaticTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<FluentTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<DerivedTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicGoal<StaticTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleAtomicGoal<FluentTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleAtomicGoal<DerivedTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(*this);
}

/**
 * Booleans
 */

void FormatterVisitor::visit(BooleanAtomicState<StaticTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanAtomicState<FluentTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanAtomicState<DerivedTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanNonempty<ConceptTag> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

void FormatterVisitor::visit(BooleanNonempty<RoleTag> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

/**
 * Numericals
 */

void FormatterVisitor::visit(NumericalCount<ConceptTag> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

void FormatterVisitor::visit(NumericalCount<RoleTag> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

void FormatterVisitor::visit(NumericalDistance constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}

/**
 * NonTerminal
 */

void FormatterVisitor::visit(NonTerminal<ConceptTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<RoleTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<BooleanTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void FormatterVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_out << constructor->get_name();
}

template void FormatterVisitor::visit_impl(NonTerminal<ConceptTag> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<RoleTag> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<BooleanTag> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<NumericalTag> constructor);

/**
 * DerivationRule
 */

void FormatterVisitor::visit(DerivationRule<ConceptTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<RoleTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<BooleanTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void FormatterVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    m_out << " ::= ";

    constructor->get_body()->accept(*this);
}

template void FormatterVisitor::visit_impl(DerivationRule<ConceptTag> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<RoleTag> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<BooleanTag> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<NumericalTag> constructor);

/**
 * SubstitutionRule
 */

void FormatterVisitor::visit(SubstitutionRule<ConceptTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<RoleTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<BooleanTag> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<NumericalTag> constructor) { visit_impl(constructor); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
void FormatterVisitor::visit_impl(SubstitutionRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    m_out << " ::= ";

    constructor->get_body()->accept(*this);
}

template void FormatterVisitor::visit_impl(SubstitutionRule<ConceptTag> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<RoleTag> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<BooleanTag> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<NumericalTag> constructor);

/**
 * Grammar
 */

void FormatterVisitor::visit(const Grammar& grammar)
{
    {
        m_out << "[start_symbols]\n";

        boost::hana::for_each(grammar.get_hana_start_symbols(),
                              [&](auto&& pair)
                              {
                                  const auto& key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;
                                  if (second.has_value())
                                  {
                                      m_out << "    " << FeatureType::name << " ::= ";

                                      second.value()->accept(*this);

                                      m_out << "\n";
                                  }
                              });
    }

    {
        m_out << "[grammar_rules]\n";

        boost::hana::for_each(grammar.get_hana_derivation_rules(),
                              [&](auto&& pair)
                              {
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      m_out << "    ";

                                      rule->accept(*this);

                                      m_out << "\n";
                                  }
                              });
    }

    {
        boost::hana::for_each(grammar.get_hana_substitution_rules(),
                              [&](auto&& pair)
                              {
                                  const auto& second = boost::hana::second(pair);

                                  for (const auto& rule : second)
                                  {
                                      m_out << "    ";

                                      rule->accept(*this);

                                      m_out << "\n";
                                  }
                              });
    }
}
}
