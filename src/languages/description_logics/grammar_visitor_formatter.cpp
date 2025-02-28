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

#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"

namespace mimir::dl::grammar
{

FormatterConstructorVisitor<Concept>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Concept>::initialize(FormatterConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                                      FormatterConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

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
void FormatterConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    m_out << keywords::concept_intersection << " ";
    constructor->get_concept_or_non_terminal_left()->accept(*m_concept_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal_right()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    m_out << keywords::concept_union << " ";
    constructor->get_concept_or_non_terminal_left()->accept(*m_concept_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal_right()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    m_out << keywords::concept_negation << " ";
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNominal constructor)
{
    m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name();
}

/**
 * Role
 */

FormatterConstructorVisitor<Role>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Role>::initialize(FormatterConstructorOrNonTerminalVisitor<Concept>& concept_or_nonterminal_visitor,
                                                   FormatterConstructorOrNonTerminalVisitor<Role>& role_or_nonterminal_visitor)
{
    m_concept_or_nonterminal_visitor = &concept_or_nonterminal_visitor;
    m_role_or_nonterminal_visitor = &role_or_nonterminal_visitor;
}

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
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_intersection << " ";
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_union << " ";
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_complement << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_inverse << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_composition << " ";
    constructor->get_role_or_non_terminal_left()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_non_terminal_right()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    assert(m_role_or_nonterminal_visitor);
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    assert(m_concept_or_nonterminal_visitor && m_role_or_nonterminal_visitor);
    m_out << keywords::role_restriction << " ";
    constructor->get_role_or_non_terminal()->accept(*m_role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    assert(m_concept_or_nonterminal_visitor);
    m_out << keywords::role_identity << " ";
    constructor->get_concept_or_non_terminal()->accept(*m_concept_or_nonterminal_visitor);
}

/**
 * ConstructorOrRoleNonTerminal
 */

template<ConceptOrRole D>
FormatterConstructorOrNonTerminalVisitor<D>::FormatterConstructorOrNonTerminalVisitor(std::ostream& out) : m_out(out)
{
}

template<ConceptOrRole D>
void FormatterConstructorOrNonTerminalVisitor<D>::initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor,
                                                             FormatterConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<ConceptOrRole D>
void FormatterConstructorOrNonTerminalVisitor<D>::visit(ConstructorOrNonTerminal<D> constructor)
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

template class FormatterConstructorOrNonTerminalVisitor<Concept>;
template class FormatterConstructorOrNonTerminalVisitor<Role>;

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
void FormatterDerivationRuleVisitor<D>::initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor,
                                                   FormatterConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<ConceptOrRole D>
void FormatterDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(m_nonterminal_visitor && m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);
    m_out << " ::= ";
    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        constructor_or_nonterminal->accept(*m_constructor_or_nonterminal_visitor);
        m_out << " ";
    }
}

template class FormatterDerivationRuleVisitor<Concept>;
template class FormatterDerivationRuleVisitor<Role>;

/**
 * Grammar
 */

FormatterGrammarVisitor::FormatterGrammarVisitor(std::ostream& out) : m_out(out) {}

void FormatterGrammarVisitor::visit(const Grammar& grammar)
{
    m_out << "[start_symbols]\n";

    boost::hana::for_each(grammar.get_start_symbols_container().get(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);
                              using KeyType = typename decltype(+key)::type;

                              if (second.has_value())
                              {
                                  m_out << "   " << KeyType::name << " = ";

                                  second.value()->accept(*boost::hana::at_key(m_start_symbol_visitor, key));

                                  m_out << "\n";
                              }
                          });

    m_out << "[grammar_rules]\n";

    boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              const auto& second = boost::hana::second(pair);

                              for (const auto& non_terminal_and_rules : second)
                              {
                                  const auto& [non_terminal, rules] = non_terminal_and_rules;

                                  m_out << "    ";

                                  for (const auto& rule : rules)
                                  {
                                      rule->accept(*boost::hana::at_key(m_derivation_rule_visitor, key));
                                  }

                                  m_out << "\n";
                              }
                          });
}

void FormatterGrammarVisitor::initialize(FormatterNonTerminalVisitor<Concept>& concept_start_symbol_visitor,
                                         FormatterNonTerminalVisitor<Role>& role_start_symbol_visitor,
                                         FormatterDerivationRuleVisitor<Concept>& concept_rule_visitor,
                                         FormatterDerivationRuleVisitor<Role>& role_rule_visitor)
{
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Concept> {}) = &concept_start_symbol_visitor;
    boost::hana::at_key(m_start_symbol_visitor, boost::hana::type<Role> {}) = &role_start_symbol_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Concept> {}) = &concept_rule_visitor;
    boost::hana::at_key(m_derivation_rule_visitor, boost::hana::type<Role> {}) = &role_rule_visitor;
}

/**
 * Printing
 */

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, Constructor<D> element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);

    if constexpr (std::is_same_v<D, Concept>)
    {
        element->accept(concept_visitor);
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        element->accept(role_visitor);
    }

    return out;
}

template std::ostream& operator<<(std::ostream& out, Constructor<Concept> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Role> element);

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, NonTerminal<D> element)
{
    auto nonterminal_visitor = FormatterNonTerminalVisitor<D>(out);

    element->accept(nonterminal_visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, NonTerminal<Concept> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Role> element);

template<ConceptOrRole D>
std::ostream& operator<<(std::ostream& out, DerivationRule<D> element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    auto concept_derivation_rule_visitor = FormatterDerivationRuleVisitor<Concept>(out);
    auto role_derivation_rule_visitor = FormatterDerivationRuleVisitor<Role>(out);
    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);

    if constexpr (std::is_same_v<D, Concept>)
    {
        element->accept(concept_derivation_rule_visitor);
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        element->accept(role_derivation_rule_visitor);
    }

    return out;
}

template std::ostream& operator<<(std::ostream& out, DerivationRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Role> element);

std::ostream& operator<<(std::ostream& out, const Grammar& element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    auto concept_derivation_rule_visitor = FormatterDerivationRuleVisitor<Concept>(out);
    auto role_derivation_rule_visitor = FormatterDerivationRuleVisitor<Role>(out);
    auto grammar_visitor = FormatterGrammarVisitor(out);
    concept_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    role_visitor.initialize(concept_or_nonterminal_visitor, role_or_nonterminal_visitor);
    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    grammar_visitor.initialize(concept_nonterminal_visitor, role_nonterminal_visitor, concept_derivation_rule_visitor, role_derivation_rule_visitor);

    element.accept(grammar_visitor);

    return out;
}

}
