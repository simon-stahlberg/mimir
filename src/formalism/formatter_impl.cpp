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

#include "formatter_impl.hpp"

#include "mimir/common/formatter.hpp"
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
std::string to_string(const T& element, F formatter)
{
    std::stringstream ss;
    if constexpr (std::is_pointer_v<std::decay_t<decltype(element)>>)
        write(*element, formatter, ss);
    else
        write(element, formatter, ss);
    return ss.str();
}

template<Formatter F, std::ranges::input_range Range>
std::vector<std::string> to_strings(const Range& range, F formatter)
{
    auto result = std::vector<std::string> {};
    if constexpr (std::ranges::sized_range<Range>)
        result.reserve(std::ranges::size(range));
    for (const auto& element : range)
        result.push_back(to_string(element, formatter));
    return result;
}

template<Formatter T>
void write(const ConjunctiveConditionImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "(and{}{}{}{})",
               element.get_literals<StaticTag>().empty() ? "" : fmt::format(" {}", fmt::join(to_strings(element.get_literals<StaticTag>(), formatter), " ")),
               element.get_literals<FluentTag>().empty() ? "" : fmt::format(" {}", fmt::join(to_strings(element.get_literals<FluentTag>(), formatter), " ")),
               element.get_literals<DerivedTag>().empty() ? "" : fmt::format(" {}", fmt::join(to_strings(element.get_literals<DerivedTag>(), formatter), " ")),
               element.get_numeric_constraints().empty() ? "" : fmt::format(" {}", fmt::join(to_strings(element.get_numeric_constraints(), formatter), " ")));
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
        fmt::print(out, "{}:parameters ({})\n", indent, fmt::join(to_strings(element.get_parameters(), formatter), " "));

    // Conditions
    fmt::print(out, "{}:precondition {}\n", indent, to_string(element.get_conjunctive_condition(), formatter));

    // Effects
    fmt::print(out, "{}:effect {}\n", indent, fmt::join(to_strings(element.get_conditional_effects(), formatter), " "));

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
        fmt::print(out, "({} {})", element.get_predicate()->get_name(), fmt::join(to_strings(element.get_terms(), formatter), " "));
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
        fmt::print(out, "(:derived ({} {})\n", predicate_name, fmt::join(to_strings(element.get_parameters(), formatter), " "));

    formatter.indent += formatter.add_indent;
    auto indent = std::string(formatter.indent, ' ');

    // Conditions
    fmt::print(out, "{}{}\n", indent, to_string(element.get_conjunctive_condition(), formatter));

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
        fmt::print(out, "{}{}\n", indent, to_string(element.get_requirements(), formatter));

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
                fmt::print(out, "{}{}\n", indent, fmt::join(to_strings(constants, formatter), " "));
            else if (types.size() > 1)
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(to_strings(constants, formatter), " "), fmt::join(to_strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(to_strings(constants, formatter), " "), fmt::join(to_strings(types, formatter), " "));
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
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(to_strings(sub_types, formatter), " "), fmt::join(to_strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(to_strings(sub_types, formatter), " "), fmt::join(to_strings(types, formatter), " "));

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
                fmt::print(out, "{}{}\n", indent, to_string(predicate, formatter));

        fmt::print(out, "{}; fluent predicates:\n", indent);
        for (const auto& predicate : element.get_predicates<FluentTag>())
            fmt::print(out, "{}{}\n", indent, to_string(predicate, formatter));

        fmt::print(out, "{}; derived predicates:\n", indent);
        for (const auto& predicate : element.get_predicates<DerivedTag>())
            fmt::print(out, "{}{}\n", indent, to_string(predicate, formatter));

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
            fmt::print(out, "{}{}\n", indent, to_string(function_skeleton, formatter));

        fmt::print(out, "{}; fluent functions:\n", indent);
        for (const auto& function_skeleton : element.get_function_skeletons<FluentTag>())
            fmt::print(out, "{}{}\n", indent, to_string(function_skeleton, formatter));

        if (element.get_auxiliary_function_skeleton().has_value())
        {
            fmt::print(out, "{}; auxiliary function:\n", indent);
            fmt::print(out, "{}{}\n", indent, to_string(element.get_auxiliary_function_skeleton().value(), formatter));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    for (const auto& action : element.get_actions())
        fmt::print(out, "{}{}\n", indent, to_string(action, formatter));

    for (const auto& axiom : element.get_axioms())
        fmt::print(out, "{}{}\n", indent, to_string(axiom, formatter));

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
               mimir::to_string(element.get_assign_operator()),
               to_string(element.get_function(), formatter),
               to_string(element.get_function_expression(), formatter));
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
               mimir::to_string(element.get_assign_operator()),
               to_string(element.get_function(), formatter),
               to_string(element.get_function_expression(), formatter));
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
               fmt::join(to_strings(element.get_literals(), formatter), " "),
               fmt::join(to_strings(element.get_fluent_numeric_effects(), formatter), " "),
               element.get_auxiliary_numeric_effect() ? " " + to_string(element.get_auxiliary_numeric_effect().value(), formatter) : "");
}

template void write(const ConjunctiveEffectImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const ConjunctiveEffectImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const ConditionalEffectImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "(forall ({}) (when {} {}))",
               fmt::join(to_strings(element.get_conjunctive_condition()->get_parameters(), formatter), " "),
               to_string(element.get_conjunctive_condition(), formatter),
               to_string(element.get_conjunctive_effect(), formatter));
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
               mimir::to_string(element.get_binary_operator()),
               to_string(element.get_left_function_expression(), formatter),
               to_string(element.get_right_function_expression(), formatter));
}

template void write(const FunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "({} {})", mimir::to_string(element.get_multi_operator()), fmt::join(to_strings(element.get_function_expressions(), formatter), " "));
}

template void write(const FunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const FunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(- {})", to_string(element.get_function_expression(), formatter));
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
        fmt::print(out, "({} {})", element.get_name(), fmt::join(to_strings(element.get_parameters(), formatter), " "));
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
        fmt::print(out, "({} {})", element.get_function_skeleton()->get_name(), fmt::join(to_strings(element.get_terms(), formatter), " "));
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
        fmt::print(out, "({} {})", element.get_predicate()->get_name(), fmt::join(to_strings(element.get_objects(), formatter), " "));
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
               mimir::to_string(element.get_binary_operator()),
               to_string(element.get_left_function_expression(), formatter),
               to_string(element.get_right_function_expression(), formatter));
}

template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionBinaryOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "({} {})", mimir::to_string(element.get_multi_operator()), fmt::join(to_strings(element.get_function_expressions(), formatter), " "));
}

template void write(const GroundFunctionExpressionMultiOperatorImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundFunctionExpressionMultiOperatorImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMinusImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(- {})", to_string(element.get_function_expression(), formatter));
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
        fmt::print(out, "({} {})", element.get_function_skeleton()->get_name(), fmt::join(to_strings(element.get_objects(), formatter), " "));
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
        fmt::print(out, "(not {})", to_string(element.get_atom(), formatter));
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
        fmt::print(out, "(not {})", to_string(element.get_atom(), formatter));
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
    fmt::print(out, "{} {}", mimir::to_string(element.get_optimization_metric()), to_string(element.get_function_expression(), formatter));
}

template void write(const OptimizationMetricImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const OptimizationMetricImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const NumericConstraintImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               mimir::to_string(element.get_binary_comparator()),
               to_string(element.get_left_function_expression(), formatter),
               to_string(element.get_right_function_expression(), formatter));
}

template void write(const NumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const NumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const GroundNumericConstraintImpl& element, T formatter, std::ostream& out)
{
    fmt::print(out,
               "({} {} {})",
               mimir::to_string(element.get_binary_comparator()),
               to_string(element.get_left_function_expression(), formatter),
               to_string(element.get_right_function_expression(), formatter));
}

template void write(const GroundNumericConstraintImpl& element, StringFormatter formatter, std::ostream& out);
template void write(const GroundNumericConstraintImpl& element, AddressFormatter formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionValueImpl<F>& element, T formatter, std::ostream& out)
{
    fmt::print(out, "(= {} {})", to_string(element.get_function(), formatter), element.get_number());
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
            fmt::print(out, "(either {})", fmt::join(to_strings(element.get_bases(), formatter), " "));
        else if (element.get_bases().size() == 1)
            fmt::print(out, "{}", to_string(element.get_bases().front(), formatter));
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
        fmt::print(out, "({} {})", element.get_name(), fmt::join(to_strings(element.get_parameters(), formatter), " "));
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
        fmt::print(out, "{}{}\n", indent, to_string(element.get_requirements(), formatter));

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
                fmt::print(out, "{}{}\n", indent, fmt::join(to_strings(objects, formatter), " "));
            else if (types.size() > 1)
                fmt::print(out, "{}{} - (either {})\n", indent, fmt::join(to_strings(objects, formatter), " "), fmt::join(to_strings(types, formatter), " "));
            else
                fmt::print(out, "{}{} - {}\n", indent, fmt::join(to_strings(objects, formatter), " "), fmt::join(to_strings(types, formatter), " "));
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
            fmt::print(out, "{}{}\n", indent, to_string(predicate, formatter));

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
            fmt::print(out, "{}{}\n", indent, to_string(literal, formatter));

        fmt::print(out, "{}; fluent literals:\n", indent);
        for (const auto& literal : element.get_initial_literals<FluentTag>())
            fmt::print(out, "{}{}\n", indent, to_string(literal, formatter));

        fmt::print(out, "{}; static function values:\n", indent);
        for (const auto& function_values : element.get_initial_function_values<StaticTag>())
            fmt::print(out, "{}{}\n", indent, to_string(function_values, formatter));

        fmt::print(out, "{}; fluent function values:\n", indent);
        for (const auto& function_values : element.get_initial_function_values<FluentTag>())
            fmt::print(out, "{}{}\n", indent, to_string(function_values, formatter));

        if (element.get_auxiliary_function_value().has_value())
        {
            fmt::print(out, "{}; auxiliary function values:\n", indent);
            fmt::print(out, "{}{}\n", indent, to_string(element.get_auxiliary_function_value().value(), formatter));
        }

        formatter.indent -= formatter.add_indent;
        indent = std::string(formatter.indent, ' ');

        fmt::print(out, "{})\n", indent);
    }

    // Goal
    if (!(element.get_goal_literals<StaticTag>().empty() && element.get_goal_literals<FluentTag>().empty() && element.get_goal_literals<DerivedTag>().empty()
          && element.get_goal_numeric_constraints().empty()))
    {
        fmt::print(
            out,
            "{}(:goal (and{}{}{}{}))\n",
            indent,
            element.get_goal_literals<StaticTag>().empty() ? "" :
                                                             fmt::format(" {}", fmt::join(to_strings(element.get_goal_literals<StaticTag>(), formatter), " ")),
            element.get_goal_literals<FluentTag>().empty() ? "" :
                                                             fmt::format(" {}", fmt::join(to_strings(element.get_goal_literals<FluentTag>(), formatter), " ")),
            element.get_goal_literals<DerivedTag>().empty() ?
                "" :
                fmt::format(" {}", fmt::join(to_strings(element.get_goal_literals<DerivedTag>(), formatter), " ")),
            element.get_goal_numeric_constraints().empty() ? "" :
                                                             fmt::format(" {}", fmt::join(to_strings(element.get_goal_numeric_constraints(), formatter), " ")));
    }

    // Metric
    if (element.get_optimization_metric().has_value())
        fmt::print(out, "{}(:metric {})\n", indent, to_string(element.get_optimization_metric().value(), formatter));

    // Axioms
    for (const auto& axiom : element.get_axioms())
        fmt::print(out, "{}{}\n", indent, to_string(axiom, formatter));

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
               fmt::join(element.get_requirements() | std::views::transform([](loki::RequirementEnum r) { return mimir::to_string(r); }), " "));
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

template<Formatter T>
void write(const std::tuple<const GroundActionImpl&, const ProblemImpl&, FullFormatterTag>& data, T formatter, std::ostream& out)
{
    const auto& [action, problem, tag] = data;

    const auto& conjunctive_condition = action.get_conjunctive_condition();
    const auto& conditional_effects = action.get_conditional_effects();

    fmt::print(out,
               "GroundAction(index={}, name={}, binding=[{}], condition={}, conditional effects=[{}])",
               action.get_index(),
               action.get_action()->get_name(),
               fmt::join(to_strings(action.get_objects(), formatter), " "),
               to_string(std::make_tuple(std::cref(*conjunctive_condition), std::cref(problem)), formatter),
               fmt::join(conditional_effects
                             | std::views::transform([&](auto&& arg) { return to_string(std::make_tuple(std::cref(*arg), std::cref(problem)), formatter); }),
                         ", "));
}

template void
write<StringFormatter>(const std::tuple<const GroundActionImpl&, const ProblemImpl&, FullFormatterTag>& data, StringFormatter formatter, std::ostream& out);
template void
write<AddressFormatter>(const std::tuple<const GroundActionImpl&, const ProblemImpl&, FullFormatterTag>& data, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const std::tuple<const GroundActionImpl&, const ProblemImpl&, PlanFormatterTag>& data, T formatter, std::ostream& out)
{
    const auto& [action, problem, tag] = data;

    fmt::print(
        out,
        "({} {})",
        action.get_action()->get_name(),
        fmt::join(to_strings(std::ranges::subrange(action.get_objects().begin(), action.get_objects().begin() + action.get_action()->get_original_arity()),
                             formatter),
                  " "));
}

template void
write<StringFormatter>(const std::tuple<const GroundActionImpl&, const ProblemImpl&, PlanFormatterTag>& data, StringFormatter formatter, std::ostream& out);
template void
write<AddressFormatter>(const std::tuple<const GroundActionImpl&, const ProblemImpl&, PlanFormatterTag>& data, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const std::tuple<const GroundAxiomImpl&, const ProblemImpl&>& data, T formatter, std::ostream& out)
{
    const auto [axiom, problem] = data;

    fmt::print(out,
               "Axiom(index={}, name={}, binding=[{}], condition={}, effect={})",
               axiom.get_index(),
               axiom.get_literal()->get_atom()->get_predicate()->get_name(),
               fmt::join(to_strings(axiom.get_objects(), formatter), ", "),
               to_string(std::make_tuple(std::cref(*axiom.get_conjunctive_condition()), std::cref(problem)), formatter),
               to_string(axiom.get_literal(), formatter));
}

template void write<StringFormatter>(const std::tuple<const GroundAxiomImpl&, const ProblemImpl&>& data, StringFormatter formatter, std::ostream& out);
template void write<AddressFormatter>(const std::tuple<const GroundAxiomImpl&, const ProblemImpl&>& data, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const std::tuple<const GroundConjunctiveConditionImpl&, const ProblemImpl&>& data, T formatter, std::ostream& out)
{
    const auto& [conjunctive_condition, problem] = data;

    const auto positive_static_precondition_indices = conjunctive_condition.get_precondition<PositiveTag, StaticTag>();
    const auto negative_static_precondition_indices = conjunctive_condition.get_precondition<NegativeTag, StaticTag>();
    const auto positive_fluent_precondition_indices = conjunctive_condition.get_precondition<PositiveTag, FluentTag>();
    const auto negative_fluent_precondition_indices = conjunctive_condition.get_precondition<NegativeTag, FluentTag>();
    const auto positive_derived_precondition_indices = conjunctive_condition.get_precondition<PositiveTag, DerivedTag>();
    const auto negative_derived_precondition_indices = conjunctive_condition.get_precondition<NegativeTag, DerivedTag>();

    auto positive_static_precondition = GroundAtomList<StaticTag> {};
    auto negative_static_precondition = GroundAtomList<StaticTag> {};
    auto positive_fluent_precondition = GroundAtomList<FluentTag> {};
    auto negative_fluent_precondition = GroundAtomList<FluentTag> {};
    auto positive_derived_precondition = GroundAtomList<DerivedTag> {};
    auto negative_derived_precondition = GroundAtomList<DerivedTag> {};
    const auto& ground_numeric_constraints = conjunctive_condition.get_numeric_constraints();

    problem.get_repositories().get_ground_atoms_from_indices<StaticTag>(positive_static_precondition_indices, positive_static_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<StaticTag>(negative_static_precondition_indices, negative_static_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(positive_fluent_precondition_indices, positive_fluent_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(negative_fluent_precondition_indices, negative_fluent_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(positive_derived_precondition_indices, positive_derived_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(negative_derived_precondition_indices, negative_derived_precondition);

    fmt::print(out,
               "GroundConjunctiveCondition(positive static preconditions=[{}], negative static preconditions=[{}], negative fluent preconditions=[{}], "
               "positive derived "
               "preconditions=[{}], negative derived preconditions=[{}], numeric constraints=[{}])",
               fmt::join(to_strings(positive_static_precondition, formatter), ", "),
               fmt::join(to_strings(negative_static_precondition, formatter), ", "),
               fmt::join(to_strings(positive_fluent_precondition, formatter), ", "),
               fmt::join(to_strings(negative_fluent_precondition, formatter), ", "),
               fmt::join(to_strings(positive_derived_precondition, formatter), ", "),
               fmt::join(to_strings(negative_derived_precondition, formatter), ", "),
               fmt::join(to_strings(ground_numeric_constraints, formatter), ", "));
}

template void
write<StringFormatter>(const std::tuple<const GroundConjunctiveConditionImpl&, const ProblemImpl&>& data, StringFormatter formatter, std::ostream& out);
template void
write<AddressFormatter>(const std::tuple<const GroundConjunctiveConditionImpl&, const ProblemImpl&>& data, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const std::tuple<const GroundConjunctiveEffectImpl&, const ProblemImpl&>& data, T formatter, std::ostream& out)
{
    const auto& [conjunctive_effect, problem] = data;

    const auto positive_literal_indices = conjunctive_effect.get_propositional_effects<PositiveTag>();
    const auto negative_literal_indices = conjunctive_effect.get_propositional_effects<NegativeTag>();

    auto positive_literals = GroundAtomList<FluentTag> {};
    auto negative_literals = GroundAtomList<FluentTag> {};
    const auto& fluent_numeric_effects = conjunctive_effect.get_fluent_numeric_effects();
    const auto& auxiliary_numeric_effect = conjunctive_effect.get_auxiliary_numeric_effect();

    problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(positive_literal_indices, positive_literals);
    problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(negative_literal_indices, negative_literals);

    fmt::print(out,
               "GroundConjunctiveEffect(delete effects=[{}], add effects=[{}], fluent numeric effects=[{}]{})",
               fmt::join(to_strings(negative_literals, formatter), ", "),
               fmt::join(to_strings(positive_literals, formatter), ", "),
               fmt::join(to_strings(fluent_numeric_effects, formatter), ", "),
               auxiliary_numeric_effect.has_value() ? ", auxiliary numeric effect=" + to_string(auxiliary_numeric_effect.value(), formatter) : "");
}

template void
write<StringFormatter>(const std::tuple<const GroundConjunctiveEffectImpl&, const ProblemImpl&>& data, StringFormatter formatter, std::ostream& out);
template void
write<AddressFormatter>(const std::tuple<const GroundConjunctiveEffectImpl&, const ProblemImpl&>& data, AddressFormatter formatter, std::ostream& out);

template<Formatter T>
void write(const std::tuple<const GroundConditionalEffectImpl&, const ProblemImpl&>& data, T formatter, std::ostream& out)
{
    const auto& [cond_effect_proxy, problem] = data;

    fmt::print(out,
               "ConditionalEffect({}, {})",
               to_string(std::make_tuple(std::cref(*cond_effect_proxy.get_conjunctive_condition()), std::cref(problem)), formatter),
               to_string(std::make_tuple(std::cref(*cond_effect_proxy.get_conjunctive_effect()), std::cref(problem)), formatter));
}

template void
write<StringFormatter>(const std::tuple<const GroundConditionalEffectImpl&, const ProblemImpl&>& data, StringFormatter formatter, std::ostream& out);
template void
write<AddressFormatter>(const std::tuple<const GroundConditionalEffectImpl&, const ProblemImpl&>& data, AddressFormatter formatter, std::ostream& out);
}