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
void FormatterVisitor::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
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
void FormatterVisitor::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
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

void FormatterVisitor::visit(BooleanAtomicState<Static> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanAtomicState<Fluent> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanAtomicState<Derived> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterVisitor::visit(BooleanNonempty<Concept> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

void FormatterVisitor::visit(BooleanNonempty<Role> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

/**
 * Numericals
 */

void FormatterVisitor::visit(NumericalCount<Concept> constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_nonterminal()->accept(*this);
}

void FormatterVisitor::visit(NumericalCount<Role> constructor)
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

void FormatterVisitor::visit(NonTerminal<Concept> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<Role> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<Boolean> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(NonTerminal<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void FormatterVisitor::visit_impl(NonTerminal<D> constructor)
{
    m_out << constructor->get_name();
}

template void FormatterVisitor::visit_impl(NonTerminal<Concept> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<Role> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<Boolean> constructor);
template void FormatterVisitor::visit_impl(NonTerminal<Numerical> constructor);

/**
 * DerivationRule
 */

void FormatterVisitor::visit(DerivationRule<Concept> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<Role> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<Boolean> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(DerivationRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void FormatterVisitor::visit_impl(DerivationRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    m_out << " ::= ";

    constructor->get_body()->accept(*this);
}

template void FormatterVisitor::visit_impl(DerivationRule<Concept> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<Role> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<Boolean> constructor);
template void FormatterVisitor::visit_impl(DerivationRule<Numerical> constructor);

/**
 * SubstitutionRule
 */

void FormatterVisitor::visit(SubstitutionRule<Concept> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<Role> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<Boolean> constructor) { visit_impl(constructor); }

void FormatterVisitor::visit(SubstitutionRule<Numerical> constructor) { visit_impl(constructor); }

template<FeatureCategory D>
void FormatterVisitor::visit_impl(SubstitutionRule<D> constructor)
{
    constructor->get_head()->accept(*this);

    m_out << " ::= ";

    constructor->get_body()->accept(*this);
}

template void FormatterVisitor::visit_impl(SubstitutionRule<Concept> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<Role> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<Boolean> constructor);
template void FormatterVisitor::visit_impl(SubstitutionRule<Numerical> constructor);

/**
 * Grammar
 */

void FormatterVisitor::visit(const Grammar& grammar)
{
    {
        m_out << "[start_symbols]\n";

        boost::hana::for_each(grammar.get_start_symbols_container().get(),
                              [&](auto&& pair)
                              {
                                  const auto& key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;
                                  if (second.has_value())
                                  {
                                      m_out << "    " << FeatureType::name << " = ";

                                      second.value()->accept(*this);

                                      m_out << "\n";
                                  }
                              });
    }

    {
        m_out << "[grammar_rules]\n";

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
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
        boost::hana::for_each(grammar.get_substitution_rules_container().get(),
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

/**
 * Printing
 */

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, Constructor<D> element)
{
    auto visitor = FormatterVisitor(out);

    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, Constructor<Concept> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Role> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Boolean> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Numerical> element);

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, NonTerminal<D> element)
{
    auto visitor = FormatterVisitor(out);

    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, NonTerminal<Concept> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Role> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Boolean> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Numerical> element);

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, DerivationRule<D> element)
{
    auto visitor = FormatterVisitor(out);

    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, DerivationRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Role> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Boolean> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Numerical> element);

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element)
{
    auto visitor = FormatterVisitor(out);

    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Role> element);
template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Boolean> element);
template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Numerical> element);

std::ostream& operator<<(std::ostream& out, const Grammar& element)
{
    auto visitor = FormatterVisitor(out);

    element.accept(visitor);

    return out;
}
}
