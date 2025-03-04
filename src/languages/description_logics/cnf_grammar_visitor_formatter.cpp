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

FormatterConstructorVisitor<Concept>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Concept>::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterConstructorVisitor<Concept>::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNominal constructor)
{
    m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name();
}

void FormatterConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    auto visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    m_out << keywords::concept_intersection << " ";
    constructor->get_concept_left()->accept(visitor);
    m_out << " ";
    constructor->get_concept_right()->accept(visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    auto visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    m_out << keywords::concept_union << " ";
    constructor->get_concept_left()->accept(visitor);
    m_out << " ";
    constructor->get_concept_right()->accept(visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    auto visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    auto concept_visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(concept_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    auto concept_visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(concept_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_role_left()->accept(role_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(role_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_role_left()->accept(role_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(role_visitor);
}

/**
 * Role
 */

FormatterConstructorVisitor<Role>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Role>::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}

void FormatterConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_intersection << " ";
    constructor->get_role_left()->accept(role_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_union << " ";
    constructor->get_role_left()->accept(role_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_composition << " ";
    constructor->get_role_left()->accept(role_visitor);
    m_out << " ";
    constructor->get_role_right()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(role_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    auto concept_visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    auto role_visitor = FormatterNonTerminalVisitor<Role>(m_out);
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(concept_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    auto concept_visitor = FormatterNonTerminalVisitor<Concept>(m_out);
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(concept_visitor);
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

template class FormatterNonTerminalVisitor<Concept>;
template class FormatterNonTerminalVisitor<Role>;

/**
 * DerivationRule
 */

template<ConceptOrRole D>
FormatterDerivationRuleVisitor<D>::FormatterDerivationRuleVisitor(std::ostream& out) : m_out(out)
{
}

template<ConceptOrRole D>
void FormatterDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    auto nonterminal_visitor = FormatterNonTerminalVisitor<D>(m_out);
    auto constructor_visitor = FormatterConstructorVisitor<D>(m_out);

    constructor->get_head()->accept(nonterminal_visitor);

    m_out << " ::= ";

    constructor->get_body()->accept(constructor_visitor);
}

template class FormatterDerivationRuleVisitor<Concept>;
template class FormatterDerivationRuleVisitor<Role>;

/**
 * SubstitutionRule
 */

template<ConceptOrRole D>
FormatterSubstitutionRuleVisitor<D>::FormatterSubstitutionRuleVisitor(std::ostream& out) : m_out(out)
{
}

template<ConceptOrRole D>
void FormatterSubstitutionRuleVisitor<D>::visit(SubstitutionRule<D> constructor)
{
    auto nonterminal_visitor = FormatterNonTerminalVisitor<D>(m_out);

    constructor->get_head()->accept(nonterminal_visitor);

    m_out << " ::= ";

    constructor->get_body()->accept(nonterminal_visitor);
}

template class FormatterSubstitutionRuleVisitor<Concept>;
template class FormatterSubstitutionRuleVisitor<Role>;

/**
 * Grammar
 */

FormatterGrammarVisitor::FormatterGrammarVisitor(std::ostream& out) : m_out(out) {}

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
                                  using ConstructorType = typename decltype(+key)::type;

                                  bool first = true;

                                  if (second.has_value())
                                  {
                                      if (!first)
                                          m_out << "\n";

                                      m_out << "    " << ConstructorType::name << " = ";

                                      auto nonterminal_visitor = FormatterNonTerminalVisitor<ConstructorType>(m_out);
                                      second.value()->accept(nonterminal_visitor);

                                      first = false;
                                  }

                                  first_0 = false;
                              });
    }

    {
        m_out << "\n[grammar_rules]\n";

        bool first_0 = true;

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using ConstructorType = typename decltype(+key)::type;

                                  if (!first_0)
                                      m_out << "\n";

                                  bool first_1 = true;

                                  for (const auto& rule : second)
                                  {
                                      if (!first_1)
                                          m_out << "\n";

                                      m_out << "    ";

                                      auto derivation_rule_visitor = FormatterDerivationRuleVisitor<ConstructorType>(m_out);
                                      rule->accept(derivation_rule_visitor);

                                      first_1 = false;
                                  }

                                  first_0 = false;
                              });

        boost::hana::for_each(grammar.get_substitution_rules().get(),
                              [&](auto&& pair)
                              {
                                  if (!first_0)
                                      m_out << "\n";

                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using ConstructorType = typename decltype(+key)::type;

                                  bool first_1 = true;

                                  for (const auto& rule : second)
                                  {
                                      if (!first_1)
                                          m_out << "\n";

                                      m_out << "    ";

                                      auto substitution_rule_visitor = FormatterSubstitutionRuleVisitor<ConstructorType>(m_out);
                                      rule->accept(substitution_rule_visitor);

                                      first_1 = false;
                                  }

                                  first_0 = false;
                              });
    }
}

/**
 * Printing
 */

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, Constructor<D> element)
{
    auto visitor = FormatterConstructorVisitor<D>(out);
    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, Constructor<Concept> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Role> element);

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, NonTerminal<D> element)
{
    auto visitor = FormatterNonTerminalVisitor<D>(out);
    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, NonTerminal<Concept> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Role> element);

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, DerivationRule<D> element)
{
    auto visitor = FormatterDerivationRuleVisitor<D>(out);
    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, DerivationRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Role> element);

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, SubstitutionRule<D> element)
{
    auto visitor = FormatterSubstitutionRuleVisitor<D>(out);
    element->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, SubstitutionRule<Role> element);

std::ostream& operator<<(std::ostream& out, const Grammar& element)
{
    auto visitor = FormatterGrammarVisitor(out);
    element.accept(visitor);

    return out;
}
}
