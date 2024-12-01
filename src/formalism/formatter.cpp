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

#include "formatter.hpp"

#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/hash.hpp"

#include <cassert>
#include <sstream>

namespace mimir
{

PDDLFormatter::PDDLFormatter(size_t indent, size_t add_indent, bool action_costs) : m_indent(indent), m_add_indent(add_indent), m_action_costs(action_costs) {}

void PDDLFormatter::write(const ActionImpl& element, std::ostream& out)
{
    out << std::string(m_indent, ' ') << "(:action " << element.get_name() << "\n";

    m_indent += m_add_indent;

    out << std::string(m_indent, ' ') << ":parameters (";
    for (size_t i = 0; i < element.get_parameters().size(); ++i)
    {
        if (i != 0)
            out << " ";
        write(*element.get_parameters()[i], out);
    }
    out << ")\n";

    out << std::string(m_indent, ' ') << ":conditions ";
    if (element.get_conditions<Static>().empty() && element.get_conditions<Fluent>().empty() && element.get_conditions<Derived>().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";
        for (const auto& condition : element.get_conditions<Static>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conditions<Fluent>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conditions<Derived>())
        {
            out << " ";
            write(*condition, out);
        }
        out << ")\n";
    }

    out << std::string(m_indent, ' ') << ":effects ";
    if (element.get_strips_effect()->get_effects().empty() && element.get_conditional_effects().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";

        for (const auto& literal : element.get_strips_effect()->get_effects())
        {
            out << " ";
            write(*literal, out);
        }

        for (const auto& effect : element.get_conditional_effects())
        {
            out << " ";
            write(*effect, out);
        }
        if (m_action_costs)
        {
            out << " (increase total-cost ";
            write(*element.get_strips_effect()->get_function_expression(), out);
            out << ")";
        }
        out << ")";  // end and
    }

    out << ")\n";  // end action

    m_indent -= m_add_indent;
}

template<PredicateTag P>
void PDDLFormatter::write(const AtomImpl<P>& element, std::ostream& out)
{
    out << "(" << element.get_predicate()->get_name();
    for (size_t i = 0; i < element.get_terms().size(); ++i)
    {
        out << " ";
        write(*element.get_terms()[i], out);
    }
    out << ")";
}

template void PDDLFormatter::write(const AtomImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const AtomImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const AtomImpl<Derived>& element, std::ostream& out);

void PDDLFormatter::write(const AxiomImpl& element, std::ostream& out)
{
    out << std::string(m_indent, ' ') << "(:derived ";
    write(*element.get_literal(), out);
    out << std::endl;

    m_indent += m_add_indent;

    out << std::string(m_indent, ' ') << "(and";
    for (const auto& condition : element.get_conditions<Static>())
    {
        out << " ";
        write(*condition, out);
    }
    for (const auto& condition : element.get_conditions<Fluent>())
    {
        out << " ";
        write(*condition, out);
    }
    for (const auto& condition : element.get_conditions<Derived>())
    {
        out << " ";
        write(*condition, out);
    }
    out << ")" << std::endl;

    m_indent -= m_add_indent;

    out << std::string(m_indent, ' ') << ")" << std::endl;
}

void PDDLFormatter::write(const DomainImpl& element, std::ostream& out)
{
    out << std::string(m_indent, ' ') << "(define (domain " << element.get_name() << ")\n";

    m_indent += m_add_indent;

    if (!element.get_requirements()->get_requirements().empty())
    {
        out << std::string(m_indent, ' ');
        write(*element.get_requirements(), out);
        out << "\n";
    }

    m_action_costs = element.get_requirements()->test(loki::RequirementEnum::ACTION_COSTS);

    if (!element.get_constants().empty())
    {
        out << std::string(m_indent, ' ') << "(:constants ";
        for (size_t i = 0; i < element.get_constants().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_constants()[i], out);
        }
        out << ")\n";
    }
    if (!(element.get_predicates<Static>().empty() && element.get_predicates<Fluent>().empty() && element.get_predicates<Derived>().empty()))
    {
        out << std::string(m_indent, ' ') << "(:predicates";
        for (const auto& predicate : element.get_predicates<Static>())
        {
            out << " ";
            write(*predicate, out);
        }
        for (const auto& predicate : element.get_predicates<Fluent>())
        {
            out << " ";
            write(*predicate, out);
        }
        for (const auto& predicate : element.get_predicates<Derived>())
        {
            out << " ";
            write(*predicate, out);
        }
        out << ")\n";
    }
    if (!element.get_functions().empty())
    {
        out << std::string(m_indent, ' ') << "(:functions ";
        for (size_t i = 0; i < element.get_functions().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_functions()[i], out);
        }
    }

    for (const auto& action : element.get_actions())
    {
        write(*action, out);
    }

    for (const auto& axiom : element.get_axioms())
    {
        write(*axiom, out);
    }

    m_indent -= m_add_indent;

    out << std::string(m_indent, ' ') << ")";
}

void PDDLFormatter::write(const EffectStripsImpl& element, std::ostream& out)
{
    for (const auto& literal : element.get_effects())
    {
        write(*literal, out);
        out << " ";
    }
}

void PDDLFormatter::write(const EffectConditionalImpl& element, std::ostream& out)
{
    if (!element.get_parameters().empty())
    {
        out << "(forall (";
        for (size_t i = 0; i < element.get_parameters().size(); ++i)
        {
            if (i != 0)
            {
                out << " ";
            }
            write(*element.get_parameters()[i], out);
        }
        out << ") ";  // end quantifiers
    }

    if (!(element.get_conditions<Static>().empty() && element.get_conditions<Fluent>().empty() && element.get_conditions<Derived>().empty()))
    {
        out << "(when (and";
        for (const auto& condition : element.get_conditions<Static>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conditions<Fluent>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conditions<Derived>())
        {
            out << " ";
            write(*condition, out);
        }
        out << ")";  // end condition and
    }

    out << " (and";
    for (const auto& effect : element.get_effects())
    {
        out << " ";
        write(*effect, out);
    }
    if (m_action_costs)
    {
        out << " (increase total-cost ";
        write(*element.get_function_expression(), out);
    }
    out << ")";  // end effect and

    if (!(element.get_conditions<Static>().empty() && element.get_conditions<Fluent>().empty() && element.get_conditions<Derived>().empty()))
    {
        out << ")";  // end effect when
    }

    if (!element.get_parameters().empty())
    {
        out << ")";  // end effect forall
    }
}

void PDDLFormatter::write(const FunctionExpressionNumberImpl& element, std::ostream& out) { out << element.get_number(); }

void PDDLFormatter::write(const FunctionExpressionBinaryOperatorImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_operator()) << " ";
    write(*element.get_left_function_expression(), out);
    out << " ";
    write(*element.get_right_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const FunctionExpressionMultiOperatorImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_multi_operator());
    assert(!element.get_function_expressions().empty());
    for (const auto& function_expression : element.get_function_expressions())
    {
        out << " ";
        write(*function_expression, out);
    }
    out << ")";
}

void PDDLFormatter::write(const FunctionExpressionMinusImpl& element, std::ostream& out)
{
    out << "(- ";
    write(*element.get_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const FunctionExpressionFunctionImpl& element, std::ostream& out) { write(*element.get_function(), out); }

void PDDLFormatter::write(const FunctionExpressionImpl& element, std::ostream& out)
{
    std::visit([this, &out](const auto& arg) { this->write(*arg, out); }, element.get_variant());
}

void PDDLFormatter::write(const FunctionSkeletonImpl& element, std::ostream& out)
{
    out << "(" << element.get_name();
    for (const auto& parameter : element.get_parameters())
    {
        out << " ";
        write(*parameter, out);
    }
    out << ")";
}

void PDDLFormatter::write(const FunctionImpl& element, std::ostream& out)
{
    if (element.get_terms().empty())
    {
        out << "(" << element.get_function_skeleton()->get_name() << ")";
    }
    else
    {
        out << "(" << element.get_function_skeleton()->get_name() << "(";
        for (size_t i = 0; i < element.get_terms().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_terms()[i], out);
        }
        out << "))";
    }
}

template<PredicateTag P>
void PDDLFormatter::write(const GroundAtomImpl<P>& element, std::ostream& out)
{
    out << "(" << element.get_predicate()->get_name();
    for (const auto& object : element.get_objects())
    {
        out << " " << object->get_name();
    }
    out << ")";
}

template void PDDLFormatter::write(const GroundAtomImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundAtomImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundAtomImpl<Derived>& element, std::ostream& out);

void PDDLFormatter::write(const GroundFunctionExpressionNumberImpl& element, std::ostream& out) { out << element.get_number(); }

void PDDLFormatter::write(const GroundFunctionExpressionBinaryOperatorImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_operator()) << " ";
    write(*element.get_left_function_expression(), out);
    out << " ";
    write(*element.get_right_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const GroundFunctionExpressionMultiOperatorImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_multi_operator());
    assert(!element.get_function_expressions().empty());
    for (const auto& function_expression : element.get_function_expressions())
    {
        out << " ";
        write(*function_expression, out);
    }
    out << ")";
}

void PDDLFormatter::write(const GroundFunctionExpressionMinusImpl& element, std::ostream& out)
{
    out << "(- ";
    write(*element.get_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl& element, std::ostream& out) { write(*element.get_function(), out); }

void PDDLFormatter::write(const GroundFunctionExpressionImpl& element, std::ostream& out)
{
    std::visit([this, &out](auto&& arg) { this->write(*arg, out); }, element.get_variant());
}

void PDDLFormatter::write(const GroundFunctionImpl& element, std::ostream& out)
{
    if (element.get_objects().empty())
    {
        out << "(" << element.get_function_skeleton()->get_name() << ")";
    }
    else
    {
        out << "(" << element.get_function_skeleton()->get_name() << "(";
        for (size_t i = 0; i < element.get_objects().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_objects()[i], out);
        }
        out << "))";
    }
}

template<PredicateTag P>
void PDDLFormatter::write(const GroundLiteralImpl<P>& element, std::ostream& out)
{
    if (element.is_negated())
    {
        out << "(not ";
        write(*element.get_atom(), out);
        out << ")";
    }
    else
    {
        write(*element.get_atom(), out);
    }
}

template void PDDLFormatter::write(const GroundLiteralImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundLiteralImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundLiteralImpl<Derived>& element, std::ostream& out);

template<PredicateTag P>
void PDDLFormatter::write(const LiteralImpl<P>& element, std::ostream& out)
{
    if (element.is_negated())
    {
        out << "(not ";
        write(*element.get_atom(), out);
        out << ")";
    }
    else
    {
        write(*element.get_atom(), out);
    }
}

template void PDDLFormatter::write(const LiteralImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const LiteralImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const LiteralImpl<Derived>& element, std::ostream& out);

void PDDLFormatter::write(const OptimizationMetricImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_optimization_metric()) << " ";
    write(*element.get_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const NumericFluentImpl& element, std::ostream& out)
{
    out << "(= ";
    write(*element.get_function(), out);
    out << " " << element.get_number() << ")";
}

void PDDLFormatter::write(const ObjectImpl& element, std::ostream& out) { out << element.get_name(); }

template<PredicateTag P>
void PDDLFormatter::write(const PredicateImpl<P>& element, std::ostream& out)
{
    out << "(" << element.get_name();
    for (const auto& parameter : element.get_parameters())
    {
        out << " ";
        write(*parameter, out);
    }
    out << ")";
}

template void PDDLFormatter::write(const PredicateImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const PredicateImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const PredicateImpl<Derived>& element, std::ostream& out);

void PDDLFormatter::write(const ProblemImpl& element, std::ostream& out)
{
    out << std::string(m_indent, ' ') << "(define (problem " << element.get_name() << ")" << std::endl;

    m_indent += m_add_indent;

    out << std::string(m_indent, ' ') << "(:domain " << element.get_domain()->get_name() << ")" << std::endl;
    if (!element.get_requirements()->get_requirements().empty())
    {
        out << std::string(m_indent, ' ');
        write(*element.get_requirements(), out);
        out << std::endl;
    }

    if (!element.get_objects().empty())
    {
        out << std::string(m_indent, ' ') << "(:objects ";
        for (size_t i = 0; i < element.get_objects().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_objects()[i], out);
        }
        out << ")" << std::endl;
    }

    if (!element.get_derived_predicates().empty())
    {
        out << std::string(m_indent, ' ') << "(:predicates ";
        for (size_t i = 0; i < element.get_derived_predicates().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_derived_predicates()[i], out);
        }
        out << ")" << std::endl;
    }

    if (!(element.get_static_initial_literals().empty() && element.get_fluent_initial_literals().empty() && element.get_numeric_fluents().empty()))
    {
        out << std::string(m_indent, ' ') << "(:init ";
        for (size_t i = 0; i < element.get_static_initial_literals().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_static_initial_literals()[i], out);
        }
        for (size_t i = 0; i < element.get_fluent_initial_literals().size(); ++i)
        {
            out << " ";
            write(*element.get_fluent_initial_literals()[i], out);
        }
        for (size_t i = 0; i < element.get_numeric_fluents().size(); ++i)
        {
            out << " ";
            write(*element.get_numeric_fluents()[i], out);
        }
    }
    out << ")" << std::endl;

    if (!(element.get_goal_condition<Static>().empty() && element.get_goal_condition<Fluent>().empty() && element.get_goal_condition<Derived>().empty()))
    {
        out << std::string(m_indent, ' ') << "(:goal ";
        out << "(and";
        for (const auto& literal : element.get_goal_condition<Static>())
        {
            out << " ";
            write(*literal, out);
        }
        for (const auto& literal : element.get_goal_condition<Fluent>())
        {
            out << " ";
            write(*literal, out);
        }
        for (const auto& literal : element.get_goal_condition<Derived>())
        {
            out << " ";
            write(*literal, out);
        }
        out << " ))";
    }

    if (element.get_optimization_metric().has_value())
    {
        out << std::string(m_indent, ' ') << "(:metric ";
        write(*element.get_optimization_metric().value(), out);
        out << ")" << std::endl;
    }

    for (const auto& axiom : element.get_axioms())
    {
        write(*axiom, out);
    }

    m_indent -= m_add_indent;

    out << std::string(m_indent, ' ') << ")";
}

void PDDLFormatter::write(const RequirementsImpl& element, std::ostream& out)
{
    out << "(:requirements ";
    int i = 0;
    for (const auto& requirement : element.get_requirements())
    {
        if (i != 0)
            out << " ";
        out << to_string(requirement);
        ++i;
    }
    out << ")";
}

void PDDLFormatter::write(const TermImpl& element, std::ostream& out)
{
    std::visit([this, &out](auto&& arg) { this->write(*arg, out); }, element.get_variant());
}

void PDDLFormatter::write(const VariableImpl& element, std::ostream& out) { out << element.get_name(); }

}