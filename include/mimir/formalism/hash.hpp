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
        return std::visit(
            [](const auto& arg)
            {
                using DecayedType = std::decay_t<decltype(arg)>;
                return UniquePDDLHasher<DecayedType>()(arg);
            },
            variant);
    }
};

template<>
struct UniquePDDLHasher<Action>
{
    size_t operator()(Action e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<Atom<P>>
{
    size_t operator()(Atom<P> e) const;
};

template<>
struct UniquePDDLHasher<Axiom>
{
    size_t operator()(Axiom e) const;
};

template<>
struct UniquePDDLHasher<Domain>
{
    size_t operator()(Domain e) const;
};

template<>
struct UniquePDDLHasher<EffectStrips>
{
    size_t operator()(EffectStrips e) const;
};

template<>
struct UniquePDDLHasher<EffectConditional>
{
    size_t operator()(EffectConditional e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpressionNumber>
{
    size_t operator()(FunctionExpressionNumber e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpressionBinaryOperator>
{
    size_t operator()(FunctionExpressionBinaryOperator e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpressionMultiOperator>
{
    size_t operator()(FunctionExpressionMultiOperator e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpressionMinus>
{
    size_t operator()(FunctionExpressionMinus e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpressionFunction>
{
    size_t operator()(FunctionExpressionFunction e) const;
};

template<>
struct UniquePDDLHasher<FunctionExpression>
{
    size_t operator()(FunctionExpression e) const;
};

template<>
struct UniquePDDLHasher<FunctionSkeleton>
{
    size_t operator()(FunctionSkeleton e) const;
};

template<>
struct UniquePDDLHasher<Function>
{
    size_t operator()(Function e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<GroundAtom<P>>
{
    size_t operator()(GroundAtom<P> e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpressionNumber>
{
    size_t operator()(GroundFunctionExpressionNumber e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpressionBinaryOperator>
{
    size_t operator()(GroundFunctionExpressionBinaryOperator e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpressionMultiOperator>
{
    size_t operator()(GroundFunctionExpressionMultiOperator e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpressionMinus>
{
    size_t operator()(GroundFunctionExpressionMinus e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpressionFunction>
{
    size_t operator()(GroundFunctionExpressionFunction e) const;
};

template<>
struct UniquePDDLHasher<GroundFunctionExpression>
{
    size_t operator()(GroundFunctionExpression e) const;
};

template<>
struct UniquePDDLHasher<GroundFunction>
{
    size_t operator()(GroundFunction e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<GroundLiteral<P>>
{
    size_t operator()(GroundLiteral<P> e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<Literal<P>>
{
    size_t operator()(Literal<P> e) const;
};

template<>
struct UniquePDDLHasher<OptimizationMetric>
{
    size_t operator()(OptimizationMetric e) const;
};

template<>
struct UniquePDDLHasher<NumericFluent>
{
    size_t operator()(NumericFluent e) const;
};

template<>
struct UniquePDDLHasher<Object>
{
    size_t operator()(Object e) const;
};

template<PredicateTag P>
struct UniquePDDLHasher<Predicate<P>>
{
    size_t operator()(Predicate<P> e) const;
};

template<>
struct UniquePDDLHasher<Problem>
{
    size_t operator()(Problem e) const;
};

template<>
struct UniquePDDLHasher<Requirements>
{
    size_t operator()(Requirements e) const;
};

template<>
struct UniquePDDLHasher<Term>
{
    size_t operator()(Term e) const;
};

template<>
struct UniquePDDLHasher<Variable>
{
    size_t operator()(Variable e) const;
};

}

#endif
