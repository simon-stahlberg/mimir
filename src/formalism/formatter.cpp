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

#include "mimir/formalism/formatter.hpp"

#include "formatter_impl.hpp"

namespace mimir
{
namespace formalism
{
std::ostream& operator<<(std::ostream& out, const ActionImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const AtomImpl<P>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const AtomImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const AtomImpl<DerivedTag>& element);

std::ostream& operator<<(std::ostream& out, const AxiomImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ConjunctiveConditionImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const DomainImpl& element) { return mimir::print(out, element); }

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const NumericEffectImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, const ConjunctiveEffectImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ConditionalEffectImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionNumberImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionBinaryOperatorImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMultiOperatorImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const FunctionExpressionMinusImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionExpressionFunctionImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, const FunctionExpressionImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const FunctionImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const FunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const FunctionImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<P>& element)
{
    return print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundAtomImpl<DerivedTag>& element);

template<IsFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<F>& element)
{
    write(element, StringFormatter(), out);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundNumericEffectImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionNumberImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionBinaryOperatorImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMultiOperatorImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionMinusImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionFunctionImpl<AuxiliaryTag>& element);

std::ostream& operator<<(std::ostream& out, const GroundFunctionExpressionImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<F>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<AuxiliaryTag>& element);

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<P>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const GroundLiteralImpl<DerivedTag>& element);

std::ostream& operator<<(std::ostream& out, const GroundNumericConstraintImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const LiteralImpl<P>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const LiteralImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const LiteralImpl<DerivedTag>& element);

std::ostream& operator<<(std::ostream& out, const OptimizationMetricImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const NumericConstraintImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ObjectImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const ParameterImpl& element) { return mimir::print(out, element); }

template<IsStaticOrFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const PredicateImpl<P>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const PredicateImpl<StaticTag>& element);
template std::ostream& operator<<(std::ostream& out, const PredicateImpl<FluentTag>& element);
template std::ostream& operator<<(std::ostream& out, const PredicateImpl<DerivedTag>& element);

std::ostream& operator<<(std::ostream& out, const ProblemImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const RequirementsImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const TermImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const TypeImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const VariableImpl& element) { return mimir::print(out, element); }

}  // end formalism

std::ostream& print(std::ostream& out, const mimir::formalism::ActionImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<P>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::AtomImpl<mimir::formalism::DerivedTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::AxiomImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::ConjunctiveConditionImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::DomainImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::NumericEffectImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::NumericEffectImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::NumericEffectImpl<mimir::formalism::AuxiliaryTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::ConjunctiveEffectImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::ConditionalEffectImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionNumberImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionBinaryOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionMultiOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionMinusImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionFunctionImpl<mimir::formalism::AuxiliaryTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::FunctionExpressionImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::FunctionSkeletonImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionSkeletonImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionSkeletonImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionSkeletonImpl<mimir::formalism::AuxiliaryTag>& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::FunctionImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::FunctionImpl<mimir::formalism::AuxiliaryTag>& element);

template<>
std::ostream&
operator<<(std::ostream& os,
           const std::tuple<const mimir::formalism::GroundActionImpl&, const mimir::formalism::ProblemImpl&, mimir::formalism::FullFormatterTag>& data)
{
    write(data, mimir::formalism::StringFormatter(), os);
    return os;
}

template<>
std::ostream&
operator<<(std::ostream& os,
           const std::tuple<const mimir::formalism::GroundActionImpl&, const mimir::formalism::ProblemImpl&, mimir::formalism::PlanFormatterTag>& data)
{
    write(data, mimir::formalism::StringFormatter(), os);
    return os;
}

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundAtomImpl<P>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::GroundAtomImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundAtomImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundAtomImpl<mimir::formalism::DerivedTag>& element);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<const formalism::GroundAxiomImpl&, const formalism::ProblemImpl&>& data)
{
    formalism::write(data, formalism::StringFormatter(), os);
    return os;
}

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<const formalism::GroundConjunctiveConditionImpl&, const formalism::ProblemImpl&>& data)
{
    formalism::write(data, formalism::StringFormatter(), os);
    return os;
}

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundConjunctiveEffectImpl&, const formalism::ProblemImpl&>& data)
{
    formalism::write(data, formalism::StringFormatter(), out);
    return out;
}

template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::GroundConditionalEffectImpl&, const formalism::ProblemImpl&>& data)
{
    formalism::write(data, formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionNumberImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionBinaryOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionMultiOperatorImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionMinusImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionFunctionImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionFunctionImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionFunctionImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionFunctionImpl<mimir::formalism::AuxiliaryTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionExpressionImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionValueImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionValueImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionValueImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionValueImpl<mimir::formalism::AuxiliaryTag>& element);

template<mimir::formalism::IsStaticOrFluentOrAuxiliaryTag F>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionImpl<F>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundFunctionImpl<mimir::formalism::AuxiliaryTag>& element);

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::GroundLiteralImpl<P>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::GroundLiteralImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundLiteralImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::GroundLiteralImpl<mimir::formalism::DerivedTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::GroundNumericConstraintImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::LiteralImpl<P>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::LiteralImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::LiteralImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::LiteralImpl<mimir::formalism::DerivedTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::OptimizationMetricImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::NumericConstraintImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::ObjectImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::ParameterImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template<mimir::formalism::IsStaticOrFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::formalism::PredicateImpl<P>& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::formalism::PredicateImpl<mimir::formalism::StaticTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::PredicateImpl<mimir::formalism::FluentTag>& element);
template std::ostream& print(std::ostream& out, const mimir::formalism::PredicateImpl<mimir::formalism::DerivedTag>& element);

std::ostream& print(std::ostream& out, const mimir::formalism::ProblemImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::RequirementsImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::TermImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::TypeImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::formalism::VariableImpl& element)
{
    write(element, mimir::formalism::StringFormatter(), out);
    return out;
}
}