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

#include "mimir/common/printers.hpp"
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
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/type.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <sstream>

namespace mimir::formalism
{

/**
 * Helpers to materialize strings
 */

template<Formatter F, typename T>
std::string string(const T& element, F formatter)
{
    std::stringstream ss;
    if constexpr (std::is_pointer_v<std::decay_t<decltype(element)>>)
        write(*element, formatter, ss);
    else
        write(element, formatter, ss);
    return ss.str();
}

template<Formatter F, std::ranges::input_range Range>
std::vector<std::string> strings(const Range& range, F formatter)
{
    auto result = std::vector<std::string> {};
    if constexpr (std::ranges::sized_range<Range>)
        result.reserve(std::ranges::size(range));
    for (const auto& element : range)
        result.push_back(string(element, formatter));
    return result;
}

template<Formatter T>
void write(const ConjunctiveConditionImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "(and{}{}{}{})",
               element.get_literals<StaticTag>().empty() ? "" : fmt::format(" {}", fmt::join(strings(element.get_literals<StaticTag>(), formatter), " ")),
               element.get_literals<FluentTag>().empty() ? "" : fmt::format(" {}", fmt::join(strings(element.get_literals<FluentTag>(), formatter), " ")),
               element.get_literals<DerivedTag>().empty() ? "" : fmt::format(" {}", fmt::join(strings(element.get_literals<DerivedTag>(), formatter), " ")),
               element.get_numeric_constraints().empty() ? "" : fmt::format(" {}", fmt::join(strings(element.get_numeric_constraints(), formatter), " ")));
}

template void write(const ConjunctiveConditionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConjunctiveConditionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ActionImpl& element, T formatter, std::ostream& out)
{
    // Header
    fmt::print(out, "(:action {}\n", element.get_name());

    formatter.indent += formatter.add_indent;
    auto indent = std::string(formatter.indent, ' ');

    // Parameters
    if (element.get_parameters().empty())
        fmt::print(out, "{}:parameters ()\n", indent);
    else
        fmt::print(out, "{}:parameters ({})\n", indent, fmt::join(strings(element.get_parameters(), formatter), " "));

    // Conditions
    fmt::print(out, "{}:precondition {}\n", indent, string(element.get_conjunctive_condition(), formatter));

    // Effects
    fmt::print(out, "{}:effect {}\n", indent, fmt::join(strings(element.get_conditional_effects(), formatter), " "));

    formatter.indent -= formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // End action
    fmt::print(out, "{})", indent);
}

template void write(const ActionImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ActionImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const AtomImpl<P>& element, T formatter, std::ostream& out)
{
    if (element.get_terms().empty())
        fmt::print(out, "({})", element.get_predicate()->get_name());
    else
        fmt::print(out, "({} {})", element.get_predicate()->get_name(), fmt::join(strings(element.get_terms(), formatter), " "));
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
    // Header line: "(:derived <pred> <params...\n"
    const auto& predicate_name = element.get_literal()->get_atom()->get_predicate()->get_name();
    if (element.get_parameters().empty())
        fmt::print(out, "(:derived ({})\n", predicate_name);
    else
        fmt::print(out, "(:derived ({} {})\n", predicate_name, fmt::join(strings(element.get_parameters(), formatter), " "));

    formatter.indent += formatter.add_indent;
    auto indent = std::string(formatter.indent, ' ');

    // Conditions
    fmt::print(out, "{}{}\n", indent, string(element.get_conjunctive_condition(), formatter));

    formatter.indent -= formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // End axiom
    fmt::print(out, "{})", indent);
}

template void write(const AxiomImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const AxiomImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const DomainImpl& element, T formatter, std::ostream& out)
{
    auto indent = std::string(formatter.indent, ' ');

    // Header
    fmt::print(out, "{}(define (domain {})\n", indent, element.get_name());

    formatter.indent += formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // Requirements
    if (!element.get_requirements()->get_requirements().empty())
        fmt::print(out, "{}{}\n", indent, string(element.get_requirements(), formatter));

    // Constants
    if (!element.get_constants().empty())
    {
        std::unordered_map<TypeList, ObjectList, loki::Hash<TypeList>> constants_by_types;
        for (const auto& constant : element.get_constants())
            constants_by_types[constant->get_bases()].push_back(constant);

        fmt::print(out, "{}(:constants\n", indent);

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        for (const auto& [types, constants] : constants_by_types)
        {
            if (!element.get_requirements()->test(loki::RequirementEnum::TYPING))
                fmt::print(out, "{}{}\n", indent, fmt::join(strings(constants, formatter), " "));
            else if (types.size() > 1)
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(strings(constants, formatter), " "), fmt::join(strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(strings(constants, formatter), " "), fmt::join(strings(types, formatter), " "));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // Types
    if (!element.get_types().empty())
    {
        std::unordered_map<TypeList, TypeList, loki::Hash<TypeList>> subtypes_by_parent_types;
        for (const auto& type : element.get_types())
            if (!type->get_bases().empty())
                subtypes_by_parent_types[type->get_bases()].push_back(type);

        fmt::print(out, "{}(:types\n", indent);

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        for (const auto& [types, sub_types] : subtypes_by_parent_types)
            if (types.size() > 1)
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(strings(sub_types, formatter), " "), fmt::join(strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(strings(sub_types, formatter), " "), fmt::join(strings(types, formatter), " "));

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // Predicates
    if (!(element.get_predicates<StaticTag>().empty() && element.get_predicates<FluentTag>().empty() && element.get_predicates<DerivedTag>().empty()))
    {
        fmt::print(out, "{}(:predicates\n", std::string(formatter.indent, ' '));

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{}; static predicates:\n", indent);
        for (const auto& predicate : element.get_predicates<StaticTag>())
            if (predicate->get_name() != "=")
                fmt::print(out, "{}{}\n", indent, string(predicate, formatter));

        fmt::print(out, "{}; fluent predicates:\n", indent);
        for (const auto& predicate : element.get_predicates<FluentTag>())
            fmt::print(out, "{}{}\n", indent, string(predicate, formatter));

        fmt::print(out, "{}; derived predicates:\n", indent);
        for (const auto& predicate : element.get_predicates<DerivedTag>())
            fmt::print(out, "{}{}\n", indent, string(predicate, formatter));

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // FunctionSkeletons
    if (!(element.get_function_skeletons<StaticTag>().empty() && element.get_function_skeletons<FluentTag>().empty()
          && !element.get_auxiliary_function_skeleton().has_value()))
    {
        fmt::print(out, "{}(:functions\n", std::string(formatter.indent, ' '));

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{}; static functions:\n", indent);
        for (const auto& function_skeleton : element.get_function_skeletons<StaticTag>())
            fmt::print(out, "{}{}\n", indent, string(function_skeleton, formatter));

        fmt::print(out, "{}; fluent functions:\n", indent);
        for (const auto& function_skeleton : element.get_function_skeletons<FluentTag>())
            fmt::print(out, "{}{}\n", indent, string(function_skeleton, formatter));

        if (element.get_auxiliary_function_skeleton().has_value())
        {
            fmt::print(out, "{}; auxiliary function:\n", indent);
            fmt::print(out, "{}{}\n", indent, string(element.get_auxiliary_function_skeleton().value(), formatter));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    for (const auto& action : element.get_actions())
        fmt::print(out, "{}{}\n", indent, string(action, formatter));

    for (const auto& axiom : element.get_axioms())
        fmt::print(out, "{}{}\n", indent, string(axiom, formatter));

    formatter.indent -= formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // End domain
    fmt::print(out, "{})", indent);
}

template void write(const DomainImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const DomainImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const NumericEffectImpl<F>& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_assign_operator()),
               string(element.get_function(), formatter),
               string(element.get_function_expression(), formatter));
}

template void write(const NumericEffectImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const NumericEffectImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const GroundNumericEffectImpl<F>& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_assign_operator()),
               string(element.get_function(), formatter),
               string(element.get_function_expression(), formatter));
}

template void write(const GroundNumericEffectImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundNumericEffectImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ConjunctiveEffectImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "{} {}{}",
               fmt::join(strings(element.get_literals(), formatter), " "),
               fmt::join(strings(element.get_fluent_numeric_effects(), formatter), " "),
               element.get_auxiliary_numeric_effect() ? " " + string(element.get_auxiliary_numeric_effect().value(), formatter) : "");
}

template void write(const ConjunctiveEffectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConjunctiveEffectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ConditionalEffectImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "(forall ({}) (when {} {}))",
               fmt::join(strings(element.get_conjunctive_condition()->get_parameters(), formatter), " "),
               string(element.get_conjunctive_condition(), formatter),
               string(element.get_conjunctive_effect(), formatter));
}

template void write(const ConditionalEffectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConditionalEffectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionNumberImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "{}", element.get_number());
}

template void write(const FunctionExpressionNumberImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionNumberImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_binary_operator()),
               string(element.get_left_function_expression(), formatter),
               string(element.get_right_function_expression(), formatter));
}

template void write(const FunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "({} {})", to_string(element.get_multi_operator()), fmt::join(strings(element.get_function_expressions(), formatter), " "));
}

template void write(const FunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(- {})", string(element.get_function_expression(), formatter));
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
    if (element.get_parameters().empty())
        fmt::print(out, "({})", element.get_name());
    else
        fmt::print(out, "({} {})", element.get_name(), fmt::join(strings(element.get_parameters(), formatter), " "));
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
        fmt::print(out, "({})", element.get_function_skeleton()->get_name());
    else
        fmt::print(out, "({} {})", element.get_function_skeleton()->get_name(), fmt::join(strings(element.get_terms(), formatter), " "));
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
    if (element.get_objects().empty())
        fmt::print(out, "({})", element.get_predicate()->get_name());
    else
        fmt::print(out, "({} {})", element.get_predicate()->get_name(), fmt::join(strings(element.get_objects(), formatter), " "));
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
    fmt::print(out, "{}", element.get_number());
}

template void write(const GroundFunctionExpressionNumberImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionNumberImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_binary_operator()),
               string(element.get_left_function_expression(), formatter),
               string(element.get_right_function_expression(), formatter));
}

template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "({} {})", to_string(element.get_multi_operator()), fmt::join(strings(element.get_function_expressions(), formatter), " "));
}

template void write(const GroundFunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(- {})", string(element.get_function_expression(), formatter));
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
        fmt::print(out, "({})", element.get_function_skeleton()->get_name());
    else
        fmt::print(out, "({} {})", element.get_function_skeleton()->get_name(), fmt::join(strings(element.get_objects(), formatter), " "));
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
        fmt::print(out, "(not {})", string(element.get_atom(), formatter));
    else
        write<T>(*element.get_atom(), formatter, out);
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
        fmt::print(out, "(not {})", string(element.get_atom(), formatter));
    else
        write<T>(*element.get_atom(), formatter, out);
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
    fmt::print(out, "{} {}", to_string(element.get_optimization_metric()), string(element.get_function_expression(), formatter));
}

template void write(const OptimizationMetricImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const OptimizationMetricImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const NumericConstraintImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_binary_comparator()),
               string(element.get_left_function_expression(), formatter),
               string(element.get_right_function_expression(), formatter));
}

template void write(const NumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundNumericConstraintImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               to_string(element.get_binary_comparator()),
               string(element.get_left_function_expression(), formatter),
               string(element.get_right_function_expression(), formatter));
}

template void write(const GroundNumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionValueImpl<F>& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(= {} {})", string(element.get_function(), formatter), element.get_number());
}

template void write(const GroundFunctionValueImpl<StaticTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<FluentTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<AuxiliaryTag>& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<StaticTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<FluentTag>& element, AddressFormatter formatter, std::ostream& out);
template void write(const GroundFunctionValueImpl<AuxiliaryTag>& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ParameterImpl& element, T formatter, std::ostream& out)
{
    write(*element.get_variable(), formatter, out);

    if (!element.get_bases().empty())
    {
        fmt::print(out, "{}", " - ");

        if (element.get_bases().size() > 1)
            fmt::print(out, "(either {})", fmt::join(strings(element.get_bases(), formatter), " "));
        else if (element.get_bases().size() == 1)
            fmt::print(out, "{}", string(element.get_bases().front(), formatter));
    }
}

template void write<StringFormatter>(const ParameterImpl& element, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const ParameterImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const PredicateImpl<P>& element, T formatter, std::ostream& out)
{
    if (element.get_parameters().empty())
        fmt::print(out, "({})", element.get_name());
    else
        fmt::print(out, "({} {})", element.get_name(), fmt::join(strings(element.get_parameters(), formatter), " "));
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
    auto indent = std::string(formatter.indent, ' ');

    // Header
    fmt::print(out, "{}(define (problem {})\n", indent, element.get_name());

    formatter.indent += formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // Domain
    fmt::print(out, "{}(:domain {})\n", indent, element.get_domain()->get_name());

    // Requirements
    if (!element.get_requirements()->get_requirements().empty())
        fmt::print(out, "{}{}\n", indent, string(element.get_requirements(), formatter));

    // Constants
    if (!element.get_objects().empty())
    {
        std::unordered_map<TypeList, ObjectList, loki::Hash<TypeList>> objects_by_types;
        for (const auto& constant : element.get_objects())
            objects_by_types[constant->get_bases()].push_back(constant);

        fmt::print(out, "{}(:objects\n", indent);

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        for (const auto& [types, objects] : objects_by_types)
        {
            if (!element.get_requirements()->test(loki::RequirementEnum::TYPING))
                fmt::print(out, "{}{}\n", indent, fmt::join(strings(objects, formatter), " "));
            else if (types.size() > 1)
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(strings(objects, formatter), " "), fmt::join(strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(strings(objects, formatter), " "), fmt::join(strings(types, formatter), " "));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // Predicates
    if (!element.get_derived_predicates().empty())
    {
        fmt::print(out, "{}(:predicates\n", std::string(formatter.indent, ' '));

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        for (const auto& predicate : element.get_derived_predicates())
            fmt::print(out, "{}{}\n", indent, string(predicate, formatter));

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    if (!(element.get_initial_literals<StaticTag>().empty() && element.get_initial_literals<FluentTag>().empty()
          && element.get_initial_function_values<StaticTag>().empty() && element.get_initial_function_values<FluentTag>().empty()
          && !element.get_auxiliary_function_value().has_value()))
    {
        fmt::print(out, "{}(:init\n", indent);

        formatter.indent += formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{}; static literals:\n", indent);
        for (const auto& literal : element.get_initial_literals<StaticTag>())
            fmt::print(out, "{}{}\n", indent, string(literal, formatter));

        fmt::print(out, "{}; fluent literals:\n", indent);
        for (const auto& literal : element.get_initial_literals<FluentTag>())
            fmt::print(out, "{}{}\n", indent, string(literal, formatter));

        fmt::print(out, "{}; static function values:\n", indent);
        for (const auto& function_values : element.get_initial_function_values<StaticTag>())
            fmt::print(out, "{}{}\n", indent, string(function_values, formatter));

        fmt::print(out, "{}; fluent function values:\n", indent);
        for (const auto& function_values : element.get_initial_function_values<FluentTag>())
            fmt::print(out, "{}{}\n", indent, string(function_values, formatter));

        if (element.get_auxiliary_function_value().has_value())
        {
            fmt::print(out, "{}; auxiliary function values:\n", indent);
            fmt::print(out, "{}{}\n", indent, string(element.get_auxiliary_function_value().value(), formatter));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // Goal
    if (!(element.get_goal_condition<StaticTag>().empty() && element.get_goal_condition<FluentTag>().empty() && element.get_goal_condition<DerivedTag>().empty()
          && element.get_numeric_goal_condition().empty()))
    {
        fmt::print(
            out,
            "{}(:goal (and{}{}{}{}))\n",
            indent,
            element.get_goal_condition<StaticTag>().empty() ? "" :
                                                              fmt::format(" {}", fmt::join(strings(element.get_goal_condition<StaticTag>(), formatter), " ")),
            element.get_goal_condition<FluentTag>().empty() ? "" :
                                                              fmt::format(" {}", fmt::join(strings(element.get_goal_condition<FluentTag>(), formatter), " ")),
            element.get_goal_condition<DerivedTag>().empty() ? "" :
                                                               fmt::format(" {}", fmt::join(strings(element.get_goal_condition<DerivedTag>(), formatter), " ")),
            element.get_numeric_goal_condition().empty() ? "" : fmt::format(" {}", fmt::join(strings(element.get_numeric_goal_condition(), formatter), " ")));
    }

    // Metric
    if (element.get_optimization_metric().has_value())
        fmt::print(out, "{}(:metric {})\n", indent, string(element.get_optimization_metric().value(), formatter));

    // Axioms
    for (const auto& axiom : element.get_axioms())
        fmt::print(out, "{}{}\n", indent, string(axiom, formatter));

    formatter.indent -= formatter.add_indent;
    indent = std::string(formatter.indent, ' ');

    // End problem
    fmt::print(out, "{})", indent);
}

template void write(const ProblemImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ProblemImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const RequirementsImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "(:requirements {})",
               fmt::join(element.get_requirements() | std::views::transform([](loki::RequirementEnum r) { return to_string(r); }), " "));
}

template void write(const RequirementsImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const RequirementsImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const TypeImpl& element, T, std::ostream& out)
{
    fmt::print(out, "{}", element.get_name());
}

template void write<StringFormatter>(const TypeImpl& element, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const TypeImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ObjectImpl& element, T, std::ostream& out)
{
    fmt::print(out, "{}", element.get_name());
}

template void write<StringFormatter>(const ObjectImpl& element, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const ObjectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const VariableImpl& element, T, std::ostream& out)
{
    fmt::print(out, "{}", element.get_name());
}

template void write<StringFormatter>(const VariableImpl& element, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const VariableImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const TermImpl& element, T formatter, std::ostream& out)
{
    std::visit([&](const auto& arg) { write<T>(*arg, formatter, out); }, element.get_variant());
}

template void write<StringFormatter>(const TermImpl& element, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const TermImpl& element, AddressFormatter formatter, std::ostream& out);

}