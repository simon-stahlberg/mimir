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
#include <loki/details/utils/observer_ptr.hpp>
#include <ranges>
#include <utility>
#include <variant>

template<>
struct std::hash<loki::ObserverPtr<const mimir::ActionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::ActionImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::AtomImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::AtomImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::AxiomImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::AxiomImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::DomainImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::DomainImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::EffectStripsImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::EffectStripsImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::EffectConditionalImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::EffectConditionalImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionExpressionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionExpressionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionSkeletonImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionSkeletonImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::FunctionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::FunctionImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::GroundAtomImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundAtomImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::GroundFunctionImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundFunctionImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::GroundLiteralImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::LiteralImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::LiteralImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::OptimizationMetricImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::OptimizationMetricImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::NumericFluentImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::NumericFluentImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::ObjectImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::ObjectImpl> ptr) const;
};

template<mimir::PredicateTag P>
struct std::hash<loki::ObserverPtr<const mimir::PredicateImpl<P>>>
{
    size_t operator()(loki::ObserverPtr<const mimir::PredicateImpl<P>> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::ProblemImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::ProblemImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::RequirementsImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::RequirementsImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::TermImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::TermImpl> ptr) const;
};

template<>
struct std::hash<loki::ObserverPtr<const mimir::VariableImpl>>
{
    size_t operator()(loki::ObserverPtr<const mimir::VariableImpl> ptr) const;
};

#endif
