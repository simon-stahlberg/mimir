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
            out << " " << *condition;
        }
        for (const auto& condition : element.get_conditions<Fluent>())
        {
            out << " " << *condition;
        }
        for (const auto& condition : element.get_conditions<Derived>())
        {
            out << " " << *condition;
        }
        out << ")\n";
    }

    out << std::string(m_indent, ' ') << ":effects ";
    if (element.get_simple_effects().empty() && element.get_conditional_effects().empty() && element.get_universal_effects().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";
        for (const auto& effect : element.get_simple_effects())
        {
            out << " " << *effect;
        }
        for (const auto& effect : element.get_conditional_effects())
        {
            out << " " << *effect;
        }
        for (const auto& effect : element.get_universal_effects())
        {
            out << " " << *effect;
        }
        if (m_action_costs)
        {
            out << " "
                << "(increase total-cost ";
            write(*element.get_function_expression(), out);
            out << ")";
        }
        out << ")";  // end and
    }

    out << ")\n";  // end action
}

template<PredicateCategory P>
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

void PDDLFormatter::write(const AxiomImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const DomainImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const EffectSimpleImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const EffectConditionalImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const EffectUniversalImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionNumberImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionBinaryOperatorImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionMultiOperatorImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionMinusImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionFunctionImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionSkeletonImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const FunctionImpl& element, std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const GroundAtomImpl<P>& element, std::ostream& out)
{
}

void PDDLFormatter::write(const GroundFunctionExpressionNumberImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionBinaryOperatorImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionMultiOperatorImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionMinusImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionImpl& element, std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const GroundLiteralImpl<P>& element, std::ostream& out)
{
}

template<PredicateCategory P>
void PDDLFormatter::write(const LiteralImpl<P>& element, std::ostream& out)
{
}

void PDDLFormatter::write(const OptimizationMetricImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const NumericFluentImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const ObjectImpl& element, std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const PredicateImpl<P>& element, std::ostream& out)
{
}

void PDDLFormatter::write(const ProblemImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const RequirementsImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const TermObjectImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const TermVariableImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const TermImpl& element, std::ostream& out) {}

void PDDLFormatter::write(const VariableImpl& element, std::ostream& out) {}

}