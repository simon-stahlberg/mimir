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

void FormatterConstructorVisitor<Concept>::initialize(HanaFormatterConstructorOrNonTerminalVisitors<Concept, Role> visitors) { m_visitors = visitors; }

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
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::concept_intersection << " ";
    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::concept_union << " ";
    constructor->get_left_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::concept_negation << " ";
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNominal constructor)
{
    m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name();
}

/**
 * Role
 */

FormatterConstructorVisitor<Role>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Role>::initialize(HanaFormatterConstructorOrNonTerminalVisitors<Concept, Role> visitors) { m_visitors = visitors; }

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
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_intersection << " ";
    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_union << " ";
    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_complement << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_inverse << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_composition << " ";
    constructor->get_left_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    m_out << keywords::role_restriction << " ";
    constructor->get_role_or_non_terminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::role_identity << " ";
    constructor->get_concept_or_non_terminal()->accept(*concept_or_nonterminal_visitor);
}

/**
 * Booleans
 */

FormatterConstructorVisitor<Boolean>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Boolean>::initialize(HanaFormatterConstructorOrNonTerminalVisitors<Concept, Role> visitors) { m_visitors = visitors; }

void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Static> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Fluent> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Derived> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}

void FormatterConstructorVisitor<Boolean>::visit(BooleanNonempty<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::numerical_distance << " ";
    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}

void FormatterConstructorVisitor<Boolean>::visit(BooleanNonempty<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::numerical_distance << " ";
    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
}

/**
 * Numericals
 */

FormatterConstructorVisitor<Numerical>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Numerical>::initialize(HanaFormatterConstructorOrNonTerminalVisitors<Concept, Role> visitors) { m_visitors = visitors; }

void FormatterConstructorVisitor<Numerical>::visit(NumericalCount<Concept> constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    assert(concept_or_nonterminal_visitor);

    m_out << keywords::numerical_distance << " ";
    constructor->get_constructor_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}

void FormatterConstructorVisitor<Numerical>::visit(NumericalCount<Role> constructor)
{
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(role_or_nonterminal_visitor);

    m_out << keywords::numerical_distance << " ";
    constructor->get_constructor_or_nonterminal()->accept(*role_or_nonterminal_visitor);
}

void FormatterConstructorVisitor<Numerical>::visit(NumericalDistance constructor)
{
    auto concept_or_nonterminal_visitor = get_visitor<Concept>(m_visitors);
    auto role_or_nonterminal_visitor = get_visitor<Role>(m_visitors);
    assert(concept_or_nonterminal_visitor && role_or_nonterminal_visitor);

    m_out << keywords::numerical_distance << " ";
    constructor->get_left_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_role_or_nonterminal()->accept(*role_or_nonterminal_visitor);
    m_out << " ";
    constructor->get_right_concept_or_nonterminal()->accept(*concept_or_nonterminal_visitor);
}

/**
 * ConstructorOrRoleNonTerminal
 */

template<FeatureCategory D>
FormatterConstructorOrNonTerminalVisitor<D>::FormatterConstructorOrNonTerminalVisitor(std::ostream& out) : m_out(out)
{
}

template<FeatureCategory D>
void FormatterConstructorOrNonTerminalVisitor<D>::initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor,
                                                             FormatterConstructorVisitor<D>& constructor_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_visitor = &constructor_visitor;
}

template<FeatureCategory D>
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
template class FormatterConstructorOrNonTerminalVisitor<Boolean>;
template class FormatterConstructorOrNonTerminalVisitor<Numerical>;

/**
 * NonTerminal
 */

template<FeatureCategory D>
FormatterNonTerminalVisitor<D>::FormatterNonTerminalVisitor(std::ostream& out) : m_out(out)
{
}

template<FeatureCategory D>
void FormatterNonTerminalVisitor<D>::visit(NonTerminal<D> constructor)
{
    m_out << constructor->get_name();
}

template class FormatterNonTerminalVisitor<Concept>;
template class FormatterNonTerminalVisitor<Role>;
template class FormatterNonTerminalVisitor<Boolean>;
template class FormatterNonTerminalVisitor<Numerical>;

/**
 * DerivationRule
 */

template<FeatureCategory D>
FormatterDerivationRuleVisitor<D>::FormatterDerivationRuleVisitor(std::ostream& out) : m_out(out)
{
}

template<FeatureCategory D>
void FormatterDerivationRuleVisitor<D>::initialize(FormatterNonTerminalVisitor<D>& nonterminal_visitor,
                                                   FormatterConstructorOrNonTerminalVisitor<D>& constructor_or_nonterminal_visitor)
{
    m_nonterminal_visitor = &nonterminal_visitor;
    m_constructor_or_nonterminal_visitor = &constructor_or_nonterminal_visitor;
}

template<FeatureCategory D>
void FormatterDerivationRuleVisitor<D>::visit(DerivationRule<D> constructor)
{
    assert(this->m_nonterminal_visitor && this->m_constructor_or_nonterminal_visitor);

    constructor->get_non_terminal()->accept(*m_nonterminal_visitor);

    m_out << " ::= ";

    bool first = true;

    for (const auto& constructor_or_nonterminal : constructor->get_constructor_or_non_terminals())
    {
        if (!first)
            m_out << " | ";

        constructor_or_nonterminal->accept(*m_constructor_or_nonterminal_visitor);

        first = false;
    }
}

template class FormatterDerivationRuleVisitor<Concept>;
template class FormatterDerivationRuleVisitor<Role>;
template class FormatterDerivationRuleVisitor<Boolean>;
template class FormatterDerivationRuleVisitor<Numerical>;

/**
 * Grammar
 */

FormatterGrammarVisitor::FormatterGrammarVisitor(std::ostream& out) : m_out(out) {}

void FormatterGrammarVisitor::visit(const Grammar& grammar)
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

                                      auto visitor = get_visitor<FeatureType>(m_start_symbol_visitors);
                                      assert(visitor);

                                      second.value()->accept(*visitor);

                                      m_out << "\n";
                                  }
                              });
    }

    {
        m_out << "[grammar_rules]\n";

        boost::hana::for_each(grammar.get_derivation_rules_container().get(),
                              [&](auto&& pair)
                              {
                                  auto key = boost::hana::first(pair);
                                  const auto& second = boost::hana::second(pair);
                                  using FeatureType = typename decltype(+key)::type;

                                  for (const auto& rule : second)
                                  {
                                      m_out << "    ";

                                      auto visitor = get_visitor<FeatureType>(m_derivation_rule_visitors);
                                      assert(visitor);

                                      rule->accept(*visitor);

                                      m_out << "\n";
                                  }
                              });
    }
}

void FormatterGrammarVisitor::initialize(HanaFormatterNonTerminalVisitors<Concept, Role, Boolean, Numerical> start_symbol_visitors,
                                         HanaFormatterDerivationRuleVisitors<Concept, Role, Boolean, Numerical> derivation_rule_visitors)
{
    m_start_symbol_visitors = start_symbol_visitors;

    m_derivation_rule_visitors = derivation_rule_visitors;
}

/**
 * Printing
 */

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, Constructor<D> element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto boolean_visitor = FormatterConstructorVisitor<Boolean>(out);
    auto numerical_visitor = FormatterConstructorVisitor<Numerical>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto boolean_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Boolean>(out);
    auto numerical_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Numerical>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    auto boolean_nonterminal_visitor = FormatterNonTerminalVisitor<Boolean>(out);
    auto numerical_nonterminal_visitor = FormatterNonTerminalVisitor<Numerical>(out);

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<Concept> {},
                               dynamic_cast<FormatterConstructorOrNonTerminalVisitor<Concept>*>(&concept_or_nonterminal_visitor)),
        boost::hana::make_pair(boost::hana::type<Role> {}, dynamic_cast<FormatterConstructorOrNonTerminalVisitor<Role>*>(&role_or_nonterminal_visitor)));

    concept_visitor.initialize(concept_and_role_nonterminal_visitors);
    role_visitor.initialize(concept_and_role_nonterminal_visitors);
    boolean_visitor.initialize(concept_and_role_nonterminal_visitors);
    numerical_visitor.initialize(concept_and_role_nonterminal_visitors);

    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);

    if constexpr (std::is_same_v<D, Concept>)
    {
        element->accept(concept_visitor);
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        element->accept(role_visitor);
    }
    else if constexpr (std::is_same_v<D, Boolean>)
    {
        element->accept(boolean_visitor);
    }
    else if constexpr (std::is_same_v<D, Numerical>)
    {
        element->accept(numerical_visitor);
    }

    return out;
}

template std::ostream& operator<<(std::ostream& out, Constructor<Concept> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Role> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Boolean> element);
template std::ostream& operator<<(std::ostream& out, Constructor<Numerical> element);

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, NonTerminal<D> element)
{
    auto nonterminal_visitor = FormatterNonTerminalVisitor<D>(out);

    element->accept(nonterminal_visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, NonTerminal<Concept> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Role> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Boolean> element);
template std::ostream& operator<<(std::ostream& out, NonTerminal<Numerical> element);

template<FeatureCategory D>
std::ostream& operator<<(std::ostream& out, DerivationRule<D> element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto boolean_visitor = FormatterConstructorVisitor<Boolean>(out);
    auto numerical_visitor = FormatterConstructorVisitor<Numerical>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto boolean_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Boolean>(out);
    auto numerical_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Numerical>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    auto boolean_nonterminal_visitor = FormatterNonTerminalVisitor<Boolean>(out);
    auto numerical_nonterminal_visitor = FormatterNonTerminalVisitor<Numerical>(out);
    auto concept_derivation_rule_visitor = FormatterDerivationRuleVisitor<Concept>(out);
    auto role_derivation_rule_visitor = FormatterDerivationRuleVisitor<Role>(out);
    auto boolean_derivation_rule_visitor = FormatterDerivationRuleVisitor<Boolean>(out);
    auto numerical_derivation_rule_visitor = FormatterDerivationRuleVisitor<Numerical>(out);

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_or_nonterminal_visitor),
                                                                       boost::hana::make_pair(boost::hana::type<Role> {}, &role_or_nonterminal_visitor));

    concept_visitor.initialize(concept_and_role_nonterminal_visitors);
    role_visitor.initialize(concept_and_role_nonterminal_visitors);
    boolean_visitor.initialize(concept_and_role_nonterminal_visitors);
    numerical_visitor.initialize(concept_and_role_nonterminal_visitors);

    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);

    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_visitor);

    if constexpr (std::is_same_v<D, Concept>)
    {
        element->accept(concept_derivation_rule_visitor);
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        element->accept(role_derivation_rule_visitor);
    }
    else if constexpr (std::is_same_v<D, Boolean>)
    {
        element->accept(boolean_derivation_rule_visitor);
    }
    else if constexpr (std::is_same_v<D, Numerical>)
    {
        element->accept(numerical_derivation_rule_visitor);
    }

    return out;
}

template std::ostream& operator<<(std::ostream& out, DerivationRule<Concept> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Role> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Boolean> element);
template std::ostream& operator<<(std::ostream& out, DerivationRule<Numerical> element);

std::ostream& operator<<(std::ostream& out, const Grammar& element)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
    auto boolean_visitor = FormatterConstructorVisitor<Boolean>(out);
    auto numerical_visitor = FormatterConstructorVisitor<Numerical>(out);
    auto concept_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Concept>(out);
    auto role_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Role>(out);
    auto boolean_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Boolean>(out);
    auto numerical_or_nonterminal_visitor = FormatterConstructorOrNonTerminalVisitor<Numerical>(out);
    auto concept_nonterminal_visitor = FormatterNonTerminalVisitor<Concept>(out);
    auto role_nonterminal_visitor = FormatterNonTerminalVisitor<Role>(out);
    auto boolean_nonterminal_visitor = FormatterNonTerminalVisitor<Boolean>(out);
    auto numerical_nonterminal_visitor = FormatterNonTerminalVisitor<Numerical>(out);
    auto concept_derivation_rule_visitor = FormatterDerivationRuleVisitor<Concept>(out);
    auto role_derivation_rule_visitor = FormatterDerivationRuleVisitor<Role>(out);
    auto boolean_derivation_rule_visitor = FormatterDerivationRuleVisitor<Boolean>(out);
    auto numerical_derivation_rule_visitor = FormatterDerivationRuleVisitor<Numerical>(out);
    auto grammar_visitor = FormatterGrammarVisitor(out);

    auto concept_and_role_nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_or_nonterminal_visitor),
                                                                       boost::hana::make_pair(boost::hana::type<Role> {}, &role_or_nonterminal_visitor));

    concept_visitor.initialize(concept_and_role_nonterminal_visitors);
    role_visitor.initialize(concept_and_role_nonterminal_visitors);
    boolean_visitor.initialize(concept_and_role_nonterminal_visitors);
    numerical_visitor.initialize(concept_and_role_nonterminal_visitors);

    concept_or_nonterminal_visitor.initialize(concept_nonterminal_visitor, concept_visitor);
    role_or_nonterminal_visitor.initialize(role_nonterminal_visitor, role_visitor);
    boolean_or_nonterminal_visitor.initialize(boolean_nonterminal_visitor, boolean_visitor);
    numerical_or_nonterminal_visitor.initialize(numerical_nonterminal_visitor, numerical_visitor);

    concept_derivation_rule_visitor.initialize(concept_nonterminal_visitor, concept_or_nonterminal_visitor);
    role_derivation_rule_visitor.initialize(role_nonterminal_visitor, role_or_nonterminal_visitor);
    boolean_derivation_rule_visitor.initialize(boolean_nonterminal_visitor, boolean_or_nonterminal_visitor);
    numerical_derivation_rule_visitor.initialize(numerical_nonterminal_visitor, numerical_or_nonterminal_visitor);

    auto nonterminal_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Role> {}, &role_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Boolean> {}, &boolean_nonterminal_visitor),
                                                      boost::hana::make_pair(boost::hana::type<Numerical> {}, &numerical_nonterminal_visitor));

    auto derivation_rule_visitors = boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Concept> {}, &concept_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Role> {}, &role_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Boolean> {}, &boolean_derivation_rule_visitor),
                                                          boost::hana::make_pair(boost::hana::type<Numerical> {}, &numerical_derivation_rule_visitor));

    grammar_visitor.initialize(nonterminal_visitors, derivation_rule_visitors);

    element.accept(grammar_visitor);

    return out;
}

}
