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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_EQUAL_TO_HPP_
#define MIMIR_FORMALISM_EQUAL_TO_HPP_

#include "mimir/formalism/declarations.hpp"

#include <functional>
#include <variant>

namespace mimir
{

/// @brief `UniquePDDLEqualTo` is used to compare newly created PDDL objects for uniqueness.
/// Since the children are unique, it suffices to compare nested pointers.
template<typename T>
struct UniquePDDLEqualTo
{
    bool operator()(const T& l, const T& r) const { return std::equal_to<T>()(l, r); }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniquePDDLEqualTo<std::variant<Ts...>>
{
    bool operator()(const std::variant<Ts...>& l, const std::variant<Ts...>& r) const
    {
        if (l.index() != r.index())
        {
            return false;  // Different types held
        }
        // Compare the held values, but only if they are of the same type
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
                {
                    return UniquePDDLEqualTo<decltype(lhs)>()(lhs, rhs);
                }
                else
                {
                    return false;  // Different types can't be equal
                }
            },
            l,
            r);
    }
};

template<>
struct UniquePDDLEqualTo<const ActionImpl*>
{
    bool operator()(const ActionImpl* l, const ActionImpl* r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<const AtomImpl<P>*>
{
    bool operator()(const AtomImpl<P>* l, const AtomImpl<P>* r) const;
};

template<>
struct UniquePDDLEqualTo<const AxiomImpl*>
{
    bool operator()(const AxiomImpl* l, const AxiomImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const DomainImpl*>
{
    bool operator()(const DomainImpl* l, const DomainImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const EffectSimpleImpl*>
{
    bool operator()(const EffectSimpleImpl* l, const EffectSimpleImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const EffectComplexImpl*>
{
    bool operator()(const EffectComplexImpl* l, const EffectComplexImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionNumberImpl&>
{
    bool operator()(const FunctionExpressionNumberImpl& l, const FunctionExpressionNumberImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionBinaryOperatorImpl&>
{
    bool operator()(const FunctionExpressionBinaryOperatorImpl& l, const FunctionExpressionBinaryOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionMultiOperatorImpl&>
{
    bool operator()(const FunctionExpressionMultiOperatorImpl& l, const FunctionExpressionMultiOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionMinusImpl&>
{
    bool operator()(const FunctionExpressionMinusImpl& l, const FunctionExpressionMinusImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionFunctionImpl&>
{
    bool operator()(const FunctionExpressionFunctionImpl& l, const FunctionExpressionFunctionImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionExpressionImpl*>
{
    bool operator()(const FunctionExpressionImpl* l, const FunctionExpressionImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionSkeletonImpl*>
{
    bool operator()(const FunctionSkeletonImpl* l, const FunctionSkeletonImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const FunctionImpl*>
{
    bool operator()(const FunctionImpl* l, const FunctionImpl* r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<const GroundAtomImpl<P>*>
{
    bool operator()(const GroundAtomImpl<P>* l, const GroundAtomImpl<P>* r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionNumberImpl&>
{
    bool operator()(const GroundFunctionExpressionNumberImpl& l, const GroundFunctionExpressionNumberImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionBinaryOperatorImpl&>
{
    bool operator()(const GroundFunctionExpressionBinaryOperatorImpl& l, const GroundFunctionExpressionBinaryOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionMultiOperatorImpl&>
{
    bool operator()(const GroundFunctionExpressionMultiOperatorImpl& l, const GroundFunctionExpressionMultiOperatorImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionMinusImpl&>
{
    bool operator()(const GroundFunctionExpressionMinusImpl& l, const GroundFunctionExpressionMinusImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionFunctionImpl&>
{
    bool operator()(const GroundFunctionExpressionFunctionImpl& l, const GroundFunctionExpressionFunctionImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionExpressionImpl*>
{
    bool operator()(const GroundFunctionExpressionImpl* l, const GroundFunctionExpressionImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const GroundFunctionImpl*>
{
    bool operator()(const GroundFunctionImpl* l, const GroundFunctionImpl* r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<const GroundLiteralImpl<P>*>
{
    bool operator()(const GroundLiteralImpl<P>* l, const GroundLiteralImpl<P>* r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<const LiteralImpl<P>*>
{
    bool operator()(const LiteralImpl<P>* l, const LiteralImpl<P>* r) const;
};

template<>
struct UniquePDDLEqualTo<const OptimizationMetricImpl*>
{
    bool operator()(const OptimizationMetricImpl* l, const OptimizationMetricImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const NumericFluentImpl*>
{
    bool operator()(const NumericFluentImpl* l, const NumericFluentImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const ObjectImpl*>
{
    bool operator()(const ObjectImpl* l, const ObjectImpl* r) const;
};

template<PredicateTag P>
struct UniquePDDLEqualTo<const PredicateImpl<P>*>
{
    bool operator()(const PredicateImpl<P>* l, const PredicateImpl<P>* r) const;
};

template<>
struct UniquePDDLEqualTo<const ProblemImpl*>
{
    bool operator()(const ProblemImpl* l, const ProblemImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const RequirementsImpl*>
{
    bool operator()(const RequirementsImpl* l, const RequirementsImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const TermObjectImpl&>
{
    bool operator()(const TermObjectImpl& l, const TermObjectImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const TermVariableImpl&>
{
    bool operator()(const TermVariableImpl& l, const TermVariableImpl& r) const;
};

template<>
struct UniquePDDLEqualTo<const TermImpl*>
{
    bool operator()(const TermImpl* l, const TermImpl* r) const;
};

template<>
struct UniquePDDLEqualTo<const VariableImpl*>
{
    bool operator()(const VariableImpl* l, const VariableImpl* r) const;
};

}

#endif
