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

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/declarations.hpp"

#include <ostream>

namespace mimir
{

namespace formalism
{
extern std::ostream& operator<<(std::ostream& out, const ActionImpl& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const AtomImpl<P>& element);

extern std::ostream& operator<<(std::ostream& out, const AxiomImpl& element);

extern std::ostream& operator<<(std::ostream& out, const ConjunctiveConditionImpl& element);

extern std::ostream& operator<<(std::ostream& out, const DomainImpl& element);

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<F>& element);
extern std::ostream& operator<<(std::ostream& out, const ConjunctiveEffectImpl& element);
extern std::ostream& operator<<(std::ostream& out, const ConditionalEffectImpl& element);

extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element);
template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<F>& element);
extern std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<F>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionImpl<F>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<P>& element);

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<F>& element);

extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element);
template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<F>& element);
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<F>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<F>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<P>& element);

extern std::ostream& operator<<(std::ostream& os, const GroundNumericConstraintImpl& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const LiteralImpl<P>& element);

extern std::ostream& operator<<(std::ostream& out, const OptimizationMetricImpl& element);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element);

extern std::ostream& operator<<(std::ostream& out, const ObjectImpl& element);

extern std::ostream& operator<<(std::ostream& out, const ParameterImpl& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const PredicateImpl<P>& element);

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, const RequirementsImpl& element);

extern std::ostream& operator<<(std::ostream& out, const TermImpl& element);

extern std::ostream& operator<<(std::ostream& out, const TypeImpl& element);

extern std::ostream& operator<<(std::ostream& out, const VariableImpl& element);
}

extern std::ostream& print(std::ostream& out, const mimir::formalism::ActionImpl& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<P>& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<P>& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::AxiomImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::ConjunctiveConditionImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::DomainImpl& element);

template<mimir::formalism::IsFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::NumericEffectImpl<F>& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::ConjunctiveEffectImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::ConditionalEffectImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionNumberImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionMultiOperatorImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionMinusImpl& element);
template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<F>& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionImpl& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionSkeletonImpl<F>& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& print(std::ostream& out, const mimir::formalism::FunctionImpl<F>& element);

template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<const mimir::formalism::GroundActionImpl&, const mimir::formalism::ProblemImpl&, formalism::FullFormatterTag>& data);

template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<const mimir::formalism::GroundActionImpl&, const mimir::formalism::ProblemImpl&, formalism::PlanFormatterTag>& data);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundAtomImpl<P>& element);

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundAxiomImpl&, const formalism::ProblemImpl&>& data);

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundConjunctiveConditionImpl&, const formalism::ProblemImpl&>& data);

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundConjunctiveEffectImpl&, const formalism::ProblemImpl&>& data);

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundConditionalEffectImpl&, const formalism::ProblemImpl&>& data);

extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionNumberImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionBinaryOperatorImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionMultiOperatorImpl& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionMinusImpl& element);
template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionFunctionImpl<F>& element);
extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionImpl& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionValueImpl<F>& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionImpl<F>& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundLiteralImpl<P>& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::GroundNumericConstraintImpl& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::LiteralImpl<P>& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::OptimizationMetricImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::NumericConstraintImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::ObjectImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::ParameterImpl& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::PredicateImpl<P>& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::ProblemImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::RequirementsImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::TermImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::TypeImpl& element);

extern std::ostream& print(std::ostream& out, const mimir::formalism::VariableImpl& element);
}

#endif
