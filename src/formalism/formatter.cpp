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

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>
#include <sstream>

namespace mimir
{

PDDLFormatter::PDDLFormatter(size_t indent, size_t add_indent, bool action_costs, bool numeric_fluents) :
    m_indent(indent),
    m_add_indent(add_indent),
    m_action_costs(action_costs),
    m_numeric_fluents(numeric_fluents)
{
}

void PDDLFormatter::write(const ConjunctiveConditionImpl& element, std::ostream& out)
{
    if (!element.get_parameters().empty())
    {
        out << "(forall (";
        for (const auto& parameter : element.get_parameters())
        {
            out << " ";
            write(*parameter, out);
        }
        out << ") ";
    }
    if (element.get_literals<Static>().empty() && element.get_literals<Fluent>().empty() && element.get_literals<Derived>().empty()
        && element.get_numeric_constraints().empty())
    {
        out << "() ";
    }
    else
    {
        out << "(and";
        for (const auto& condition : element.get_literals<Static>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_literals<Fluent>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_literals<Derived>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_numeric_constraints())
        {
            out << " ";
            write(*condition, out);
        }
        out << ")";  // end and
    }
    if (!element.get_parameters().empty())
    {
        out << ")";  // end forall
    }
}

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
    if (element.get_conjunctive_condition()->get_literals<Static>().empty() && element.get_conjunctive_condition()->get_literals<Fluent>().empty()
        && element.get_conjunctive_condition()->get_literals<Derived>().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<Static>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<Fluent>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<Derived>())
        {
            out << " ";
            write(*condition, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_numeric_constraints())
        {
            out << " ";
            write(*condition, out);
        }
        out << ")\n";
    }

    out << std::string(m_indent, ' ') << ":effects ";
    if (element.get_conjunctive_effect()->get_literals().empty() && element.get_conjunctive_effect()->get_fluent_numeric_effects().empty()
        && !element.get_conjunctive_effect()->get_auxiliary_numeric_effect().has_value() && element.get_conditional_effects().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and ";

        write(*element.get_conjunctive_effect(), out);

        for (const auto& effect : element.get_conditional_effects())
        {
            out << " ";
            write(*effect, out);
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
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<Static>())
    {
        out << " ";
        write(*condition, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<Fluent>())
    {
        out << " ";
        write(*condition, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<Derived>())
    {
        out << " ";
        write(*condition, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_numeric_constraints())
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
    m_numeric_fluents = element.get_requirements()->test(loki::RequirementEnum::NUMERIC_FLUENTS);

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
        out << std::string(m_indent, ' ') << "(:predicates ";
        m_indent += m_add_indent;
        if (!element.get_predicates<Static>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; static predicates:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<Static>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<Static>()[i], out);
        }
        if (!element.get_predicates<Fluent>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; fluent predicates:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<Fluent>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<Fluent>()[i], out);
        }
        if (!element.get_predicates<Derived>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; derived predicates:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<Derived>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<Derived>()[i], out);
        }
        m_indent -= m_add_indent;
        out << "\n" << std::string(m_indent, ' ') << ")\n";
    }
    if (!(element.get_functions<Static>().empty() && element.get_functions<Fluent>().empty() && !element.get_auxiliary_function().has_value()))
    {
        out << std::string(m_indent, ' ') << "(:functions ";
        m_indent += m_add_indent;
        if (!element.get_functions<Static>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; static functions:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_functions<Static>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_functions<Static>()[i], out);
        }
        if (!element.get_functions<Fluent>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; fluent functions:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_functions<Fluent>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_functions<Fluent>()[i], out);
        }
        if (element.get_auxiliary_function().has_value())
        {
            out << "\n" << std::string(m_indent, ' ') << "; auxiliary function:\n" << std::string(m_indent, ' ');
            write(*element.get_auxiliary_function().value(), out);
        }

        m_indent -= m_add_indent;
        out << "\n" << std::string(m_indent, ' ') << ")\n";
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

template<DynamicFunctionTag F>
void PDDLFormatter::write(const NumericEffectImpl<F>& element, std::ostream& out)
{
    out << "(" << to_string(element.get_assign_operator()) << " ";
    write(*element.get_function(), out);
    out << " ";
    write(*element.get_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const ConjunctiveEffectImpl& element, std::ostream& out)
{
    for (const auto& literal : element.get_literals())
    {
        write(*literal, out);
        out << " ";
    }
    for (const auto& numeric_effect : element.get_fluent_numeric_effects())
    {
        write(*numeric_effect, out);
        out << " ";
    }
    if (element.get_auxiliary_numeric_effect().has_value())
    {
        write(*element.get_auxiliary_numeric_effect().value(), out);
        out << " ";
    }
}

void PDDLFormatter::write(const ConditionalEffectImpl& element, std::ostream& out)
{
    out << "(forall (";
    for (size_t i = 0; i < element.get_conjunctive_condition()->get_parameters().size(); ++i)
    {
        if (i != 0)
            out << " ";
        write(*element.get_conjunctive_condition()->get_parameters()[i], out);
    }
    out << ") ";

    write(*element.get_conjunctive_condition(), out);
    out << " ";
    write(*element.get_conjunctive_effect(), out);

    out << ")";  // end forall
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

template<FunctionTag F>
void PDDLFormatter::write(const FunctionExpressionFunctionImpl<F>& element, std::ostream& out)
{
    write(*element.get_function(), out);
}

template void PDDLFormatter::write(const FunctionExpressionFunctionImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionExpressionFunctionImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionExpressionFunctionImpl<Auxiliary>& element, std::ostream& out);

void PDDLFormatter::write(const FunctionExpressionImpl& element, std::ostream& out)
{
    std::visit([this, &out](const auto& arg) { this->write(*arg, out); }, element.get_variant());
}

template<FunctionTag F>
void PDDLFormatter::write(const FunctionSkeletonImpl<F>& element, std::ostream& out)
{
    out << "(" << element.get_name();
    for (const auto& parameter : element.get_parameters())
    {
        out << " ";
        write(*parameter, out);
    }
    out << ")";
}

template void PDDLFormatter::write(const FunctionSkeletonImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionSkeletonImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionSkeletonImpl<Auxiliary>& element, std::ostream& out);

template<FunctionTag F>
void PDDLFormatter::write(const FunctionImpl<F>& element, std::ostream& out)
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

template void PDDLFormatter::write(const FunctionImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const FunctionImpl<Auxiliary>& element, std::ostream& out);

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

template<FunctionTag F>
void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl<F>& element, std::ostream& out)
{
    write(*element.get_function(), out);
}

template void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl<Auxiliary>& element, std::ostream& out);

void PDDLFormatter::write(const GroundFunctionExpressionImpl& element, std::ostream& out)
{
    std::visit([this, &out](auto&& arg) { this->write(*arg, out); }, element.get_variant());
}

template<FunctionTag F>
void PDDLFormatter::write(const GroundFunctionImpl<F>& element, std::ostream& out)
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

template void PDDLFormatter::write(const GroundFunctionImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionImpl<Auxiliary>& element, std::ostream& out);

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

void PDDLFormatter::write(const NumericConstraintImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_comparator()) << " ";
    write(*element.get_left_function_expression(), out);
    out << " ";
    write(*element.get_right_function_expression(), out);
    out << ")";
}

void PDDLFormatter::write(const GroundNumericConstraintImpl& element, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_comparator()) << " ";
    write(*element.get_left_function_expression(), out);
    out << " ";
    write(*element.get_right_function_expression(), out);
    out << ")";
}

template<FunctionTag F>
void PDDLFormatter::write(const GroundFunctionValueImpl<F>& element, std::ostream& out)
{
    out << "(= ";
    write(*element.get_function(), out);
    out << " " << element.get_number() << ")";
}

template void PDDLFormatter::write(const GroundFunctionValueImpl<Static>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionValueImpl<Fluent>& element, std::ostream& out);
template void PDDLFormatter::write(const GroundFunctionValueImpl<Auxiliary>& element, std::ostream& out);

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
        out << std::string(m_indent, ' ') << "(:objects";
        for (const auto& object : element.get_objects())
        {
            out << " ";
            write(*object, out);
        }
        out << ")" << std::endl;
    }

    if (!element.get_derived_predicates().empty())
    {
        out << std::string(m_indent, ' ') << "(:predicates";
        for (const auto& predicate : element.get_derived_predicates())
        {
            out << " ";
            write(*predicate, out);
        }
        out << ")" << std::endl;
    }

    if (!(element.get_static_initial_literals().empty() && element.get_fluent_initial_literals().empty() && element.get_function_values<Static>().empty()
          && element.get_function_values<Fluent>().empty() && !element.get_auxiliary_function_value().has_value()))
    {
        out << std::string(m_indent, ' ') << "(:init ";
        m_indent += m_add_indent;

        if (!element.get_static_initial_literals().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; static literals:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_static_initial_literals().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_static_initial_literals()[i], out);
        }
        if (!element.get_fluent_initial_literals().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; fluent literals:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_fluent_initial_literals().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_fluent_initial_literals()[i], out);
        }
        if (!element.get_function_values<Static>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; static function values:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_function_values<Static>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_function_values<Static>()[i], out);
        }
        if (!element.get_function_values<Fluent>().empty())
        {
            out << "\n" << std::string(m_indent, ' ') << "; fluent function values:\n" << std::string(m_indent, ' ');
        }
        for (size_t i = 0; i < element.get_function_values<Fluent>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_function_values<Fluent>()[i], out);
        }
        if (element.get_auxiliary_function_value().has_value())
        {
            out << "\n" << std::string(m_indent, ' ') << "; auxiliary function value:\n" << std::string(m_indent, ' ');
            write(*element.get_auxiliary_function_value().value(), out);
        }

        m_indent -= m_add_indent;
        out << "\n" << std::string(m_indent, ' ') << ")\n";
    }

    if (!(element.get_goal_condition<Static>().empty() && element.get_goal_condition<Fluent>().empty() && element.get_goal_condition<Derived>().empty()
          && element.get_numeric_goal_condition().empty()))
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
        for (const auto& constraint : element.get_numeric_goal_condition())
        {
            out << " ";
            write(*constraint, out);
        }
        out << "))" << std::endl;
    }

    out << std::string(m_indent, ' ') << "(:metric ";
    write(*element.get_optimization_metric(), out);
    out << ")" << std::endl;

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