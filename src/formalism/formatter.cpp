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
#include "mimir/formalism/ground_effects.hpp"
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

namespace mimir::formalism
{

template<Formatter T>
void write(const ConjunctiveConditionImpl& element, T formatter, std::ostream& out)
{
    if (!element.get_parameters().empty())
    {
        out << "(forall (";
        for (const auto& parameter : element.get_parameters())
        {
            out << " ";
            write(*parameter, formatter, out);
        }
        out << ") ";
    }
    if (element.get_literals<StaticTag>().empty() && element.get_literals<FluentTag>().empty() && element.get_literals<DerivedTag>().empty()
        && element.get_numeric_constraints().empty())
    {
        out << "() ";
    }
    else
    {
        out << "(and";
        for (const auto& condition : element.get_literals<StaticTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_literals<FluentTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_literals<DerivedTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_numeric_constraints())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        out << ")";  // end and
    }
    if (!element.get_parameters().empty())
    {
        out << ")";  // end forall
    }
}

template void write(const ConjunctiveConditionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConjunctiveConditionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ActionImpl& element, T formatter, std::ostream& out)
{
    out << std::string(formatter.indent, ' ') << "(:action " << element.get_name() << "\n";

    formatter.indent += formatter.add_indent;

    out << std::string(formatter.indent, ' ') << ":parameters (";
    for (size_t i = 0; i < element.get_parameters().size(); ++i)
    {
        if (i != 0)
            out << " ";
        write(*element.get_parameters()[i], formatter, out);
    }
    out << ")\n";

    out << std::string(formatter.indent, ' ') << ":conditions ";
    if (element.get_conjunctive_condition()->get_literals<StaticTag>().empty() && element.get_conjunctive_condition()->get_literals<FluentTag>().empty()
        && element.get_conjunctive_condition()->get_literals<DerivedTag>().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and";
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<StaticTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<FluentTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_literals<DerivedTag>())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        for (const auto& condition : element.get_conjunctive_condition()->get_numeric_constraints())
        {
            out << " ";
            write(*condition, formatter, out);
        }
        out << ")\n";
    }

    out << std::string(formatter.indent, ' ') << ":effects ";
    if (element.get_conjunctive_effect()->get_literals().empty() && element.get_conjunctive_effect()->get_fluent_numeric_effects().empty()
        && !element.get_conjunctive_effect()->get_auxiliary_numeric_effect().has_value() && element.get_conditional_effects().empty())
    {
        out << "()\n";
    }
    else
    {
        out << "(and ";

        write(*element.get_conjunctive_effect(), formatter, out);

        for (const auto& effect : element.get_conditional_effects())
        {
            out << " ";
            write(*effect, formatter, out);
        }

        out << ")";  // end and
    }

    out << ")\n";  // end action
}

template void write(const ActionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ActionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const AtomImpl<P>& element, T formatter, std::ostream& out)
{
    out << "(" << element.get_predicate()->get_name();
    for (size_t i = 0; i < element.get_terms().size(); ++i)
    {
        out << " ";
        write(*element.get_terms()[i], formatter, out);
    }
    out << ")";
}

template void write(const AtomImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const AtomImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const AtomImpl<DerivedTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const AtomImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const AtomImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const AtomImpl<DerivedTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const AxiomImpl& element, T formatter, std::ostream& out)
{
    out << std::string(formatter.indent, ' ') << "(:derived ";
    write(*element.get_literal(), formatter, out);
    out << std::endl;

    formatter.indent += formatter.add_indent;

    out << std::string(formatter.indent, ' ') << "(and";
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<StaticTag>())
    {
        out << " ";
        write(*condition, formatter, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<FluentTag>())
    {
        out << " ";
        write(*condition, formatter, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_literals<DerivedTag>())
    {
        out << " ";
        write(*condition, formatter, out);
    }
    for (const auto& condition : element.get_conjunctive_condition()->get_numeric_constraints())
    {
        out << " ";
        write(*condition, formatter, out);
    }
    out << ")" << std::endl;

    formatter.indent -= formatter.add_indent;

    out << std::string(formatter.indent, ' ') << ")" << std::endl;
}

template void write(const AxiomImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const AxiomImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const DomainImpl& element, T formatter, std::ostream& out)
{
    out << std::string(formatter.indent, ' ') << "(define (domain " << element.get_name() << ")\n";

    formatter.indent += formatter.add_indent;

    if (!element.get_requirements()->get_requirements().empty())
    {
        out << std::string(formatter.indent, ' ');
        write(*element.get_requirements(), formatter, out);
        out << "\n";
    }

    if (!element.get_constants().empty())
    {
        out << std::string(formatter.indent, ' ') << "(:constants ";
        for (size_t i = 0; i < element.get_constants().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_constants()[i], formatter, out);
        }
        out << ")\n";
    }
    if (!(element.get_predicates<StaticTag>().empty() && element.get_predicates<FluentTag>().empty() && element.get_predicates<DerivedTag>().empty()))
    {
        out << std::string(formatter.indent, ' ') << "(:predicates ";
        formatter.indent += formatter.add_indent;
        if (!element.get_predicates<StaticTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; static predicates:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<StaticTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<StaticTag>()[i], formatter, out);
        }
        if (!element.get_predicates<FluentTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; fluent predicates:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<FluentTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<FluentTag>()[i], formatter, out);
        }
        if (!element.get_predicates<DerivedTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; derived predicates:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_predicates<DerivedTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_predicates<DerivedTag>()[i], formatter, out);
        }

        formatter.indent -= formatter.add_indent;

        out << "\n" << std::string(formatter.indent, ' ') << ")\n";
    }
    if (!(element.get_function_skeletons<StaticTag>().empty() && element.get_function_skeletons<FluentTag>().empty()
          && !element.get_auxiliary_function_skeleton().has_value()))
    {
        out << std::string(formatter.indent, ' ') << "(:functions ";
        formatter.indent += formatter.add_indent;
        if (!element.get_function_skeletons<StaticTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; static functions:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_function_skeletons<StaticTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_function_skeletons<StaticTag>()[i], formatter, out);
        }
        if (!element.get_function_skeletons<FluentTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; fluent functions:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_function_skeletons<FluentTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_function_skeletons<FluentTag>()[i], formatter, out);
        }
        if (element.get_auxiliary_function_skeleton().has_value())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; auxiliary function:\n" << std::string(formatter.indent, ' ');
            write(*element.get_auxiliary_function_skeleton().value(), formatter, out);
        }

        formatter.indent -= formatter.add_indent;

        out << "\n" << std::string(formatter.indent, ' ') << ")\n";
    }

    for (const auto& action : element.get_actions())
    {
        write(*action, formatter, out);
    }

    for (const auto& axiom : element.get_axioms())
    {
        write(*axiom, formatter, out);
    }

    formatter.indent -= formatter.add_indent;

    out << std::string(formatter.indent, ' ') << ")";
}

template void write(const DomainImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const DomainImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const NumericEffectImpl<F>& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_assign_operator()) << " ";
    write(*element.get_function(), formatter, out);
    out << " ";
    write(*element.get_function_expression(), formatter, out);
    out << ")";
}

template void write(const NumericEffectImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const GroundNumericEffectImpl<F>& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_assign_operator()) << " ";
    write(*element.get_function(), formatter, out);
    out << " ";
    write(*element.get_function_expression(), formatter, out);
    out << ")";
}

template void write(const GroundNumericEffectImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ConjunctiveEffectImpl& element, T formatter, std::ostream& out)
{
    for (const auto& literal : element.get_literals())
    {
        write(*literal, formatter, out);
        out << " ";
    }
    for (const auto& numeric_effect : element.get_fluent_numeric_effects())
    {
        write(*numeric_effect, formatter, out);
        out << " ";
    }
    if (element.get_auxiliary_numeric_effect().has_value())
    {
        write(*element.get_auxiliary_numeric_effect().value(), formatter, out);
        out << " ";
    }
}

template void write(const ConjunctiveEffectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConjunctiveEffectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ConditionalEffectImpl& element, T formatter, std::ostream& out)
{
    out << "(forall (";
    for (size_t i = 0; i < element.get_conjunctive_condition()->get_parameters().size(); ++i)
    {
        if (i != 0)
            out << " ";
        write(*element.get_conjunctive_condition()->get_parameters()[i], formatter, out);
    }
    out << ") ";

    write(*element.get_conjunctive_condition(), formatter, out);
    out << " ";
    write(*element.get_conjunctive_effect(), formatter, out);

    out << ")";  // end forall
}

template void write(const ConditionalEffectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConditionalEffectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionNumberImpl& element, T formatter, std::ostream& out)
{
    out << element.get_number();
}

template void write(const FunctionExpressionNumberImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionNumberImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_operator()) << " ";
    write(*element.get_left_function_expression(), formatter, out);
    out << " ";
    write(*element.get_right_function_expression(), formatter, out);
    out << ")";
}

template void write(const FunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_multi_operator());
    assert(!element.get_function_expressions().empty());
    for (const auto& function_expression : element.get_function_expressions())
    {
        out << " ";
        write(*function_expression, formatter, out);
    }
    out << ")";
}

template void write(const FunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    out << "(- ";
    write(*element.get_function_expression(), formatter, out);
    out << ")";
}

template void write(const FunctionExpressionMinusImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionMinusImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionExpressionFunctionImpl<F>& element, T formatter, std::ostream& out)
{
    write(*element.get_function(), formatter, out);
}

template void write(const FunctionExpressionFunctionImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionFunctionImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionFunctionImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionFunctionImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionFunctionImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionFunctionImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionImpl& element, T formatter, std::ostream& out)
{
    std::visit([&](const auto& arg) { write(*arg, formatter, out); }, element.get_variant());
}

template void write(const FunctionExpressionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionSkeletonImpl<F>& element, T formatter, std::ostream& out)
{
    out << "(" << element.get_name();
    for (const auto& parameter : element.get_parameters())
    {
        out << " ";
        write(*parameter, formatter, out);
    }
    out << ")";
}

template void write(const FunctionSkeletonImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionSkeletonImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionSkeletonImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionSkeletonImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionSkeletonImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionSkeletonImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionImpl<F>& element, T formatter, std::ostream& out)
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
            write(*element.get_terms()[i], formatter, out);
        }
        out << "))";
    }
}

template void write(const FunctionImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const FunctionImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const GroundAtomImpl<P>& element, T formatter, std::ostream& out)
{
    out << "(" << element.get_predicate()->get_name();
    for (const auto& object : element.get_objects())
    {
        out << " " << object->get_name();
    }
    out << ")";
}

template void write(const GroundAtomImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundAtomImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundAtomImpl<DerivedTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundAtomImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundAtomImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundAtomImpl<DerivedTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionNumberImpl& element, T formatter, std::ostream& out)
{
    out << element.get_number();
}

template void write(const GroundFunctionExpressionNumberImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionNumberImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_operator()) << " ";
    write(*element.get_left_function_expression(), formatter, out);
    out << " ";
    write(*element.get_right_function_expression(), formatter, out);
    out << ")";
}

template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_multi_operator());
    assert(!element.get_function_expressions().empty());
    for (const auto& function_expression : element.get_function_expressions())
    {
        out << " ";
        write(*function_expression, formatter, out);
    }
    out << ")";
}

template void write(const GroundFunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    out << "(- ";
    write(*element.get_function_expression(), formatter, out);
    out << ")";
}

template void write(const GroundFunctionExpressionMinusImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionMinusImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionExpressionFunctionImpl<F>& element, T formatter, std::ostream& out)
{
    write(*element.get_function(), formatter, out);
}

template void write(const GroundFunctionExpressionFunctionImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionFunctionImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionFunctionImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionFunctionImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionFunctionImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionFunctionImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionImpl& element, T formatter, std::ostream& out)
{
    std::visit([&](auto&& arg) { write(*arg, formatter, out); }, element.get_variant());
}

template void write(const GroundFunctionExpressionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionImpl<F>& element, T formatter, std::ostream& out)
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
            write(*element.get_objects()[i], formatter, out);
        }
        out << "))";
    }
}

template void write(const GroundFunctionImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const GroundLiteralImpl<P>& element, T formatter, std::ostream& out)
{
    if (!element.get_polarity())
    {
        out << "(not ";
        write(*element.get_atom(), formatter, out);
        out << ")";
    }
    else
    {
        write(*element.get_atom(), formatter, out);
    }
}

template void write(const GroundLiteralImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundLiteralImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundLiteralImpl<DerivedTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundLiteralImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundLiteralImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundLiteralImpl<DerivedTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const LiteralImpl<P>& element, T formatter, std::ostream& out)
{
    if (!element.get_polarity())
    {
        out << "(not ";
        write(*element.get_atom(), formatter, out);
        out << ")";
    }
    else
    {
        write(*element.get_atom(), formatter, out);
    }
}

template void write(const LiteralImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const LiteralImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const LiteralImpl<DerivedTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const LiteralImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const LiteralImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const LiteralImpl<DerivedTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const OptimizationMetricImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_optimization_metric()) << " ";
    write(*element.get_function_expression(), formatter, out);
    out << ")";
}

template void write(const OptimizationMetricImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const OptimizationMetricImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const NumericConstraintImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_comparator()) << " ";
    write(*element.get_left_function_expression(), formatter, out);
    out << " ";
    write(*element.get_right_function_expression(), formatter, out);
    out << ")";
}

template void write(const NumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundNumericConstraintImpl& element, T formatter, std::ostream& out)
{
    out << "(" << to_string(element.get_binary_comparator()) << " ";
    write(*element.get_left_function_expression(), formatter, out);
    out << " ";
    write(*element.get_right_function_expression(), formatter, out);
    out << ")";
}

template void write(const GroundNumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionValueImpl<F>& element, T formatter, std::ostream& out)
{
    out << "(= ";
    write(*element.get_function(), formatter, out);
    out << " " << element.get_number() << ")";
}

template void write(const GroundFunctionValueImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ObjectImpl& element, T formatter, std::ostream& out)
{
    out << element.get_name();
}

template void write(const ObjectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ObjectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const PredicateImpl<P>& element, T formatter, std::ostream& out)
{
    out << "(" << element.get_name();
    for (const auto& parameter : element.get_parameters())
    {
        out << " ";
        write(*parameter, formatter, out);
    }
    out << ")";
}

template void write(const PredicateImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const PredicateImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const PredicateImpl<DerivedTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const PredicateImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const PredicateImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const PredicateImpl<DerivedTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ProblemImpl& element, T formatter, std::ostream& out)
{
    out << std::string(formatter.indent, ' ') << "(define (problem " << element.get_name() << ")" << std::endl;

    formatter.indent += formatter.add_indent;

    out << std::string(formatter.indent, ' ') << "(:domain " << element.get_domain()->get_name() << ")" << std::endl;
    if (!element.get_requirements()->get_requirements().empty())
    {
        out << std::string(formatter.indent, ' ');
        write(*element.get_requirements(), formatter, out);
        out << std::endl;
    }

    if (!element.get_objects().empty())
    {
        out << std::string(formatter.indent, ' ') << "(:objects";
        for (const auto& object : element.get_objects())
        {
            out << " ";
            write(*object, formatter, out);
        }
        out << ")" << std::endl;
    }

    if (!element.get_derived_predicates().empty())
    {
        out << std::string(formatter.indent, ' ') << "(:predicates";
        for (const auto& predicate : element.get_derived_predicates())
        {
            out << " ";
            write(*predicate, formatter, out);
        }
        out << ")" << std::endl;
    }

    if (!(element.get_initial_literals<StaticTag>().empty() && element.get_initial_literals<FluentTag>().empty()
          && element.get_initial_function_values<StaticTag>().empty() && element.get_initial_function_values<FluentTag>().empty()
          && !element.get_auxiliary_function_value().has_value()))
    {
        out << std::string(formatter.indent, ' ') << "(:init ";
        formatter.indent += formatter.add_indent;

        if (!element.get_initial_literals<StaticTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; static literals:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_initial_literals<StaticTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_initial_literals<StaticTag>()[i], formatter, out);
        }
        if (!element.get_initial_literals<FluentTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; fluent literals:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_initial_literals<FluentTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_initial_literals<FluentTag>()[i], formatter, out);
        }
        if (!element.get_initial_function_values<StaticTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; static function values:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_initial_function_values<StaticTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_initial_function_values<StaticTag>()[i], formatter, out);
        }
        if (!element.get_initial_function_values<FluentTag>().empty())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; fluent function values:\n" << std::string(formatter.indent, ' ');
        }
        for (size_t i = 0; i < element.get_initial_function_values<FluentTag>().size(); ++i)
        {
            if (i != 0)
                out << " ";
            write(*element.get_initial_function_values<FluentTag>()[i], formatter, out);
        }
        if (element.get_auxiliary_function_value().has_value())
        {
            out << "\n" << std::string(formatter.indent, ' ') << "; auxiliary function value:\n" << std::string(formatter.indent, ' ');
            write(*element.get_auxiliary_function_value().value(), formatter, out);
        }

        formatter.indent -= formatter.add_indent;

        out << "\n" << std::string(formatter.indent, ' ') << ")\n";
    }

    if (!(element.get_goal_condition<StaticTag>().empty() && element.get_goal_condition<FluentTag>().empty() && element.get_goal_condition<DerivedTag>().empty()
          && element.get_numeric_goal_condition().empty()))
    {
        out << std::string(formatter.indent, ' ') << "(:goal ";
        out << "(and";
        for (const auto& literal : element.get_goal_condition<StaticTag>())
        {
            out << " ";
            write(*literal, formatter, out);
        }
        for (const auto& literal : element.get_goal_condition<FluentTag>())
        {
            out << " ";
            write(*literal, formatter, out);
        }
        for (const auto& literal : element.get_goal_condition<DerivedTag>())
        {
            out << " ";
            write(*literal, formatter, out);
        }
        for (const auto& constraint : element.get_numeric_goal_condition())
        {
            out << " ";
            write(*constraint, formatter, out);
        }
        out << "))" << std::endl;
    }

    if (element.get_optimization_metric().has_value())
    {
        out << std::string(formatter.indent, ' ') << "(:metric ";
        write(*element.get_optimization_metric().value(), formatter, out);
        out << ")" << std::endl;
    }

    for (const auto& axiom : element.get_axioms())
    {
        write(*axiom, formatter, out);
    }

    formatter.indent -= formatter.add_indent;

    out << std::string(formatter.indent, ' ') << ")";
}

template void write(const ProblemImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ProblemImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const RequirementsImpl& element, T formatter, std::ostream& out)
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

template void write(const RequirementsImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const RequirementsImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const TermImpl& element, T formatter, std::ostream& out)
{
    std::visit([&](auto&& arg) { write(*arg, formatter, out); }, element.get_variant());
}

template void write(const TermImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const TermImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const VariableImpl& element, T formatter, std::ostream& out)
{
    out << element.get_name();
}

template void write(const VariableImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const VariableImpl& element, AddressFormatter formatter, std::ostream& out);

}