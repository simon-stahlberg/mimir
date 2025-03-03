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

namespace mimir::dl::cnf_grammar
{

/**
 * Concept
 */

FormatterConstructorVisitor<Concept, Primitive>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept, Primitive>::visit(ConceptNominal constructor)
{
    m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name();
}

FormatterConstructorVisitor<Concept, Composite>::FormatterConstructorVisitor(std::ostream& out) :
    m_out(out),
    m_concept_nonterminal_visitor(out),
    m_role_nonterminal_visitor(out)
{
}

void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_concept_left()->accept(m_concept_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_right()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_concept_left()->accept(m_concept_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_right()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_role_left()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept, Composite>::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_role_left()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(m_role_nonterminal_visitor);
}

/**
 * Role
 */

FormatterConstructorVisitor<Role, Primitive>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Role, Primitive>::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role, Primitive>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}

FormatterConstructorVisitor<Role, Composite>::FormatterConstructorVisitor(std::ostream& out) :
    m_out(out),
    m_concept_nonterminal_visitor(out),
    m_role_nonterminal_visitor(out)
{
}

void FormatterConstructorVisitor<Role, Composite>::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_role_left()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_role_left()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_role_left()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(m_role_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept()->accept(m_concept_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role, Composite>::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(m_concept_nonterminal_visitor);
}

/**
 * NonTerminal
 */

template<ConceptOrRole D>
FormatterNonTerminalVisitor<D>::FormatterNonTerminalVisitor(std::ostream& out) : m_out(out)
{
}

template<ConceptOrRole D>
void FormatterNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_out << constructor->get_name();
}

/**
 * DerivationRule
 */

template<ConceptOrRole D, PrimitiveOrComposite C>
FormatterDerivationRuleVisitor<D, C>::FormatterDerivationRuleVisitor(std::ostream& out) : m_out(out), m_nonterminal_visitor(out), m_constructor_visitor(out)
{
}

template<ConceptOrRole D, PrimitiveOrComposite C>
void FormatterDerivationRuleVisitor<D, C>::visit(DerivationRule<D, C> constructor)
{
    constructor->get_head()->accept(m_nonterminal_visitor);

    m_out << " ::= ";

    constructor->get_body()->accept(m_constructor_visitor);
}

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
FormatterSubstitutionRuleVisitor<D>::FormatterSubstitutionRuleVisitor(std::ostream& out) : m_out(out), m_nonterminal_visitor(out)
{
}

template<ConceptOrRole D>
void FormatterSubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> constructor)
{
    constructor->get_head()->accept(m_nonterminal_visitor);

    m_out << " ::= ";

    constructor->get_body()->accept(m_nonterminal_visitor);
}

/**
 * Grammar
 */

FormatterGrammarVisitor::FormatterGrammarVisitor(std::ostream& out) :
    m_out(out),
    m_start_symbol_visitor(boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, FormatterNonTerminalVisitor<Concept>(out)),
                                                 boost::hana::make_pair(boost::hana::type<Role> {}, FormatterNonTerminalVisitor<Role>(out)))),
    m_derivation_rule_visitor(boost::hana::make_map(
        boost::hana::make_pair(
            boost::hana::type<Concept> {},
            boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Primitive> {}, FormatterDerivationRuleVisitor<Concept, Primitive>(out)),
                                  boost::hana::make_pair(boost::hana::type<Composite> {}, FormatterDerivationRuleVisitor<Concept, Composite>(out)))),
        boost::hana::make_pair(
            boost::hana::type<Role> {},
            boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Primitive> {}, FormatterDerivationRuleVisitor<Role, Primitive>(out)),
                                  boost::hana::make_pair(boost::hana::type<Composite> {}, FormatterDerivationRuleVisitor<Role, Composite>(out))))))
{
}

void FormatterGrammarVisitor::visit(const Grammar& grammar)
{
    {
        m_out << "[start_symbols]\n";

        bool first_0 = true;

        boost::hana::for_each(grammar.get_start_symbols_container().get(),
                              [&](auto&& pair)
                              {
                                  if (!first_0)
                                  {
                                      m_out << "\n";
                                  }

                                  const auto& key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using KeyType = typename decltype(+key)::type;

                                  bool first = true;

                                  if (second.has_value())
                                  {
                                      if (!first)
                                          m_out << "\n";

                                      m_out << "    " << KeyType::name << " = ";

                                      second.value()->accept(boost::hana::at_key(m_start_symbol_visitor, key));

                                      first = false;
                                  }

                                  first_0 = false;
                              });
    }

    {
        m_out << "\n[grammar_rules]\n";

        bool first_0 = true;
    }
}

/**
 * Printing
 */

template<ConceptOrRole D, PrimitiveOrComposite C>
std::ostream& operator<<(std::ostream& out, Constructor<D, C> element)
{
    auto visitor = FormatterConstructorVisitor<D, C>(out);
    element->accept(visitor);
}

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, NonTerminal<D> element)
{
    auto visitor = FormatterNonTerminalVisitor<D>(out);
    element->accept(visitor);
}

template<ConceptOrRole D, PrimitiveOrComposite C>
std::ostream& operator<<(std::ostream& out, DerivationRule<D, C> element)
{
    auto visitor = FormatterDerivationRuleVisitor<D, C>(out);
    element->accept(visitor);
}

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element)
{
    auto visitor = FormatterSubstitutionRuleVisitor<D>(out);
    element->accept(visitor);
}

std::ostream& operator<<(std::ostream& out, const Grammar& element)
{
    auto visitor = FormatterGrammarVisitor(out);
    element.accept(visitor);
}
}
