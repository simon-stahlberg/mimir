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

#ifndef MIMIR_FORMALISM_HASH_HPP_
#define MIMIR_FORMALISM_HASH_HPP_

#include "mimir/formalism/declarations.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <ranges>
#include <utility>
#include <variant>

namespace mimir
{

/// @brief `UniquePDDLEqualTo` is used to compare newly created PDDL objects for uniqueness.
/// Since the children are unique, it suffices to create a combined hash from nested pointers.
template<typename T>
struct UniquePDDLHasher
{
    size_t operator()(const T& element) const { return std::hash<T>()(element); }
};

struct UniquePDDLHashCombiner
{
public:
    template<typename T>
    void operator()(size_t& seed, const T& value) const
    {
        seed ^= UniquePDDLHasher<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    void operator()(size_t& seed, const std::size_t& value) const { seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); }

    template<typename... Types>
    size_t operator()(const Types&... args) const
    {
        size_t seed = 0;
        ((*this)(seed, args), ...);
        return seed;
    }
};

/// Spezialization for std::ranges::forward_range.
template<typename ForwardRange>
    requires std::ranges::forward_range<ForwardRange>
struct UniquePDDLHasher<ForwardRange>
{
    size_t operator()(const ForwardRange& range) const
    {
        std::size_t aggregated_hash = 0;
        for (const auto& item : range)
        {
            UniquePDDLHashCombiner()(aggregated_hash, item);
        }
        return aggregated_hash;
    }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniquePDDLHasher<std::variant<Ts...>>
{
    size_t operator()(const std::variant<Ts...>& variant) const
    {
        return std::visit([](const auto& arg) { return UniquePDDLHasher<decltype(arg)>()(arg); }, variant);
    }
};

template<>
struct UniquePDDLHasher<const ActionImpl*>
{
    size_t operator()(const ActionImpl* e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<const AtomImpl<P>*>
{
    size_t operator()(const AtomImpl<P>* e) const;
};

template<>
struct UniquePDDLHasher<const AxiomImpl*>
{
    size_t operator()(const AxiomImpl* e) const;
};

template<>
struct UniquePDDLHasher<const DomainImpl*>
{
    size_t operator()(const DomainImpl* e) const;
};

template<>
struct UniquePDDLHasher<const EffectSimpleImpl*>
{
    size_t operator()(const EffectSimpleImpl* e) const;
};

template<>
struct UniquePDDLHasher<const EffectComplexImpl*>
{
    size_t operator()(const EffectComplexImpl* e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionNumberImpl&>
{
    size_t operator()(const FunctionExpressionNumberImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionBinaryOperatorImpl&>
{
    size_t operator()(const FunctionExpressionBinaryOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionMultiOperatorImpl&>
{
    size_t operator()(const FunctionExpressionMultiOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionMinusImpl&>
{
    size_t operator()(const FunctionExpressionMinusImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionFunctionImpl&>
{
    size_t operator()(const FunctionExpressionFunctionImpl& e) const;
};

template<>
struct UniquePDDLHasher<const FunctionExpressionImpl*>
{
    size_t operator()(const FunctionExpressionImpl* e) const;
};

template<>
struct UniquePDDLHasher<const FunctionSkeletonImpl*>
{
    size_t operator()(const FunctionSkeletonImpl* e) const;
};

template<>
struct UniquePDDLHasher<const FunctionImpl*>
{
    size_t operator()(const FunctionImpl* e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<const GroundAtomImpl<P>*>
{
    size_t operator()(const GroundAtomImpl<P>* e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionNumberImpl&>
{
    size_t operator()(const GroundFunctionExpressionNumberImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionBinaryOperatorImpl&>
{
    size_t operator()(const GroundFunctionExpressionBinaryOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionMultiOperatorImpl&>
{
    size_t operator()(const GroundFunctionExpressionMultiOperatorImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionMinusImpl&>
{
    size_t operator()(const GroundFunctionExpressionMinusImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionFunctionImpl&>
{
    size_t operator()(const GroundFunctionExpressionFunctionImpl& e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionExpressionImpl*>
{
    size_t operator()(const GroundFunctionExpressionImpl* e) const;
};

template<>
struct UniquePDDLHasher<const GroundFunctionImpl*>
{
    size_t operator()(const GroundFunctionImpl* e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<const GroundLiteralImpl<P>*>
{
    size_t operator()(const GroundLiteralImpl<P>* e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<const LiteralImpl<P>*>
{
    size_t operator()(const LiteralImpl<P>* e) const;
};

template<>
struct UniquePDDLHasher<const OptimizationMetricImpl*>
{
    size_t operator()(const OptimizationMetricImpl* e) const;
};

template<>
struct UniquePDDLHasher<const NumericFluentImpl*>
{
    size_t operator()(const NumericFluentImpl* e) const;
};

template<>
struct UniquePDDLHasher<const ObjectImpl*>
{
    size_t operator()(const ObjectImpl* e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<const PredicateImpl<P>*>
{
    size_t operator()(const PredicateImpl<P>* e) const;
};

template<>
struct UniquePDDLHasher<const ProblemImpl*>
{
    size_t operator()(const ProblemImpl* e) const;
};

template<>
struct UniquePDDLHasher<const RequirementsImpl*>
{
    size_t operator()(const RequirementsImpl* e) const;
};

template<>
struct UniquePDDLHasher<const TermObjectImpl&>
{
    size_t operator()(const TermObjectImpl& e) const;
};

template<>
struct UniquePDDLHasher<const TermVariableImpl&>
{
    size_t operator()(const TermVariableImpl& e) const;
};

template<>
struct UniquePDDLHasher<const TermImpl*>
{
    size_t operator()(const TermImpl* e) const;
};

template<>
struct UniquePDDLHasher<const VariableImpl*>
{
    size_t operator()(const VariableImpl* e) const;
};

}

#endif
