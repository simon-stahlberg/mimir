/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_FORMATTER_HPP_
#define MIMIR_FORMALISM_FORMATTER_HPP_

#include "mimir/formalism/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <ostream>

namespace mimir::formalism
{

/// @brief `StringFormatter` is supposed to print parsable string-representation.
struct StringFormatter
{
    size_t indent = 0;
    size_t add_indent = 4;
};
/// @brief `AddressFormatter` is supposed to print addresses of structures for debugging purposes.
struct AddressFormatter
{
    size_t indent = 0;
    size_t add_indent = 4;
};

template<typename T>
concept Formatter = std::same_as<T, StringFormatter> || std::same_as<T, AddressFormatter>;

template<Formatter T>
void write(const ConjunctiveConditionImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const ActionImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const AtomImpl<P>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const AxiomImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const DomainImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const NumericEffectImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T, IsFluentOrAuxiliaryTag F>
void write(const GroundNumericEffectImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const ConjunctiveEffectImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const ConditionalEffectImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionNumberImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionMinusImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionExpressionFunctionImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const FunctionExpressionImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionSkeletonImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const FunctionImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const GroundAtomImpl<P>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionNumberImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionBinaryOperatorImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMultiOperatorImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionMinusImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionExpressionFunctionImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundFunctionExpressionImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const GroundLiteralImpl<P>& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const LiteralImpl<P>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const OptimizationMetricImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const NumericConstraintImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const GroundNumericConstraintImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrAuxiliaryTag F>
void write(const GroundFunctionValueImpl<F>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const ObjectImpl& element, T formatter, std::ostream& out);

template<Formatter T, IsStaticOrFluentOrDerivedTag P>
void write(const PredicateImpl<P>& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const ProblemImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const RequirementsImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const TermImpl& element, T formatter, std::ostream& out);

template<Formatter T>
void write(const VariableImpl& element, T formatter, std::ostream& out);

}

#endif