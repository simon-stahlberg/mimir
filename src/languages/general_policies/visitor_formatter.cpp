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

#include "mimir/languages/general_policies/visitor_formatter.hpp"

#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/general_policies/conditions.hpp"
#include "mimir/languages/general_policies/effects.hpp"
#include "mimir/languages/general_policies/general_policy.hpp"
#include "mimir/languages/general_policies/keywords.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/rule.hpp"

namespace mimir::languages::general_policies
{

FormatterVisitor::FormatterVisitor(std::ostream& out) : m_out(out) {}

void FormatterVisitor::visit(PositiveBooleanCondition effect) { m_out << keywords::positive_boolean_condition << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(NegativeBooleanCondition effect) { m_out << keywords::negative_boolean_condition << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(GreaterNumericalCondition effect) { m_out << keywords::greater_numerical_condition << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(EqualNumericalCondition effect) { m_out << keywords::equal_numerical_condition << " " << effect->get_feature()->get_name(); }

void FormatterVisitor::visit(PositiveBooleanEffect effect) { m_out << keywords::positive_boolean_effect << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(NegativeBooleanEffect effect) { m_out << keywords::negative_boolean_effect << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(UnchangedBooleanEffect effect) { m_out << keywords::unchanged_boolean_effect << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(IncreaseNumericalEffect effect) { m_out << keywords::increase_numerical_effect << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(DecreaseNumericalEffect effect) { m_out << keywords::decrease_numerical_effect << " " << effect->get_feature()->get_name(); }
void FormatterVisitor::visit(UnchangedNumericalEffect effect) { m_out << keywords::unchanged_numerical_effect << " " << effect->get_feature()->get_name(); }

void FormatterVisitor::visit(NamedFeature<dl::ConceptTag> feature)
{
    m_out << feature->get_name() << " ::= ";
    auto visitor = dl::FormatterVisitor(m_out);
    feature->get_feature()->accept(visitor);
}
void FormatterVisitor::visit(NamedFeature<dl::RoleTag> feature)
{
    m_out << feature->get_name() << " ::= ";
    auto visitor = dl::FormatterVisitor(m_out);
    feature->get_feature()->accept(visitor);
}
void FormatterVisitor::visit(NamedFeature<dl::BooleanTag> feature)
{
    m_out << feature->get_name() << " ::= ";
    auto visitor = dl::FormatterVisitor(m_out);
    feature->get_feature()->accept(visitor);
}
void FormatterVisitor::visit(NamedFeature<dl::NumericalTag> feature)
{
    m_out << feature->get_name() << " ::= ";
    auto visitor = dl::FormatterVisitor(m_out);
    feature->get_feature()->accept(visitor);
}

void FormatterVisitor::visit(Rule rule)
{
    m_out << "{ ";
    bool first = true;
    for (const auto& condition : rule->get_conditions())
    {
        if (!first)
            m_out << ", ";
        condition->accept(*this);
        first = false;
    }
    m_out << " } -> { ";
    for (const auto& effect : rule->get_effects())
    {
        if (!first)
            m_out << ", ";
        effect->accept(*this);
        first = false;
    }
    m_out << " }";
}

void FormatterVisitor::visit(GeneralPolicy policy)
{
    m_out << "[boolean_features]" << "\n";
    for (const auto& boolean : policy->get_features<dl::BooleanTag>())
    {
        boolean->accept(*this);
        m_out << "\n";
    }

    m_out << "[numerical_features]" << "\n";
    for (const auto& numerical : policy->get_features<dl::NumericalTag>())
    {
        numerical->accept(*this);
        m_out << "\n";
    }

    m_out << "[policy_rules]" << "\n";
    for (const auto& rule : policy->get_rules())
    {
        rule->accept(*this);
        m_out << "\n";
    }
}

std::ostream& operator<<(std::ostream& out, Condition condition)
{
    auto visitor = FormatterVisitor(out);
    condition->accept(visitor);
    return out;
}

std::ostream& operator<<(std::ostream& out, Effect effect)
{
    auto visitor = FormatterVisitor(out);
    effect->accept(visitor);
    return out;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, NamedFeature<D> feature)
{
    auto visitor = FormatterVisitor(out);
    feature->accept(visitor);
    return out;
}

template std::ostream& operator<<(std::ostream& out, NamedFeature<dl::ConceptTag> feature);
template std::ostream& operator<<(std::ostream& out, NamedFeature<dl::RoleTag> feature);
template std::ostream& operator<<(std::ostream& out, NamedFeature<dl::BooleanTag> feature);
template std::ostream& operator<<(std::ostream& out, NamedFeature<dl::NumericalTag> feature);

std::ostream& operator<<(std::ostream& out, Rule rule)
{
    auto visitor = FormatterVisitor(out);
    rule->accept(visitor);
    return out;
}

std::ostream& operator<<(std::ostream& out, GeneralPolicy policy)
{
    auto visitor = FormatterVisitor(out);
    policy->accept(visitor);
    return out;
}

}
