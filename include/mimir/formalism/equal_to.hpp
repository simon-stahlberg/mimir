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
#include <loki/details/utils/observer_ptr.hpp>
#include <variant>

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::ActionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::ActionImpl> lhs, loki::ObserverPtr<const mimir::ActionImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::AtomImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::AtomImpl<P>> lhs, loki::ObserverPtr<const mimir::AtomImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::AxiomImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::AxiomImpl> lhs, loki::ObserverPtr<const mimir::AxiomImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::DomainImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::DomainImpl> lhs, loki::ObserverPtr<const mimir::DomainImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::EffectStripsImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::EffectStripsImpl> lhs, loki::ObserverPtr<const mimir::EffectStripsImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::EffectConditionalImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::EffectConditionalImpl> lhs, loki::ObserverPtr<const mimir::EffectConditionalImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> lhs,
                    loki::ObserverPtr<const mimir::ExistentiallyQuantifiedConjunctiveConditionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> lhs, loki::ObserverPtr<const mimir::FunctionExpressionNumberImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> lhs,
                    loki::ObserverPtr<const mimir::FunctionExpressionBinaryOperatorImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> lhs,
                    loki::ObserverPtr<const mimir::FunctionExpressionMultiOperatorImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> lhs, loki::ObserverPtr<const mimir::FunctionExpressionMinusImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> lhs,
                    loki::ObserverPtr<const mimir::FunctionExpressionFunctionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionExpressionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionExpressionImpl> lhs, loki::ObserverPtr<const mimir::FunctionExpressionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionSkeletonImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionSkeletonImpl> lhs, loki::ObserverPtr<const mimir::FunctionSkeletonImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::FunctionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::FunctionImpl> lhs, loki::ObserverPtr<const mimir::FunctionImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundAtomImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundAtomImpl<P>> lhs, loki::ObserverPtr<const mimir::GroundAtomImpl<P>> rhs) const;
};
template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> lhs,
                    loki::ObserverPtr<const mimir::GroundFunctionExpressionNumberImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> lhs,
                    loki::ObserverPtr<const mimir::GroundFunctionExpressionBinaryOperatorImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> lhs,
                    loki::ObserverPtr<const mimir::GroundFunctionExpressionMultiOperatorImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> lhs,
                    loki::ObserverPtr<const mimir::GroundFunctionExpressionMinusImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> lhs,
                    loki::ObserverPtr<const mimir::GroundFunctionExpressionFunctionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> lhs, loki::ObserverPtr<const mimir::GroundFunctionExpressionImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundFunctionImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundFunctionImpl> lhs, loki::ObserverPtr<const mimir::GroundFunctionImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::GroundLiteralImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> lhs, loki::ObserverPtr<const mimir::GroundLiteralImpl<P>> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::LiteralImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::LiteralImpl<P>> lhs, loki::ObserverPtr<const mimir::LiteralImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::OptimizationMetricImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::OptimizationMetricImpl> lhs, loki::ObserverPtr<const mimir::OptimizationMetricImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::NumericFluentImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::NumericFluentImpl> lhs, loki::ObserverPtr<const mimir::NumericFluentImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::ObjectImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::ObjectImpl> lhs, loki::ObserverPtr<const mimir::ObjectImpl> rhs) const;
};

template<mimir::PredicateTag P>
struct std::equal_to<loki::ObserverPtr<const mimir::PredicateImpl<P>>>
{
    bool operator()(loki::ObserverPtr<const mimir::PredicateImpl<P>> lhs, loki::ObserverPtr<const mimir::PredicateImpl<P>> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::ProblemImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::ProblemImpl> lhs, loki::ObserverPtr<const mimir::ProblemImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::RequirementsImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::RequirementsImpl> lhs, loki::ObserverPtr<const mimir::RequirementsImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::TermImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::TermImpl> lhs, loki::ObserverPtr<const mimir::TermImpl> rhs) const;
};

template<>
struct std::equal_to<loki::ObserverPtr<const mimir::VariableImpl>>
{
    bool operator()(loki::ObserverPtr<const mimir::VariableImpl> lhs, loki::ObserverPtr<const mimir::VariableImpl> rhs) const;
};

#endif
