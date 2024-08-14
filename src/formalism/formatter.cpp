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

#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/hash.hpp"

#include <cassert>
#include <sstream>

namespace mimir
{

PDDLFormatter::PDDLFormatter(size_t indent, size_t add_indent, bool action_costs = false) :
    m_indent(indent),
    m_add_indent(add_indent),
    m_action_costs(action_costs)
{
}

void PDDLFormatter::write(const ActionImpl& element, std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const AtomImpl<P>& element, std::ostream& out)
{
}

void PDDLFormatter::write(const AxiomImpl& element std::ostream& out) {}

void PDDLFormatter::write(const DomainImpl& element std::ostream& out) {}

void PDDLFormatter::write(const EffectSimpleImpl& element std::ostream& out) {}

void PDDLFormatter::write(const EffectConditionalImpl& element std::ostream& out) {}

void PDDLFormatter::write(const EffectUniversalImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionNumberImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionBinaryOperatorImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionMultiOperatorImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionMinusImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionFunctionImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionExpressionImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionSkeletonImpl& element std::ostream& out) {}

void PDDLFormatter::write(const FunctionImpl& element std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const GroundAtomImpl<P>& element std::ostream& out)
{
}

void PDDLFormatter::write(const GroundFunctionExpressionNumberImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionBinaryOperatorImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionMultiOperatorImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionMinusImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionFunctionImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionExpressionImpl& element std::ostream& out) {}

void PDDLFormatter::write(const GroundFunctionImpl& element std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const GroundLiteralImpl<P>& element std::ostream& out)
{
}

template<PredicateCategory P>
void PDDLFormatter::write(const LiteralImpl<P>& element std::ostream& out)
{
}

void PDDLFormatter::write(const OptimizationMetricImpl& element std::ostream& out) {}

void PDDLFormatter::write(const NumericFluentImpl& element std::ostream& out) {}

void PDDLFormatter::write(const ObjectImpl& element std::ostream& out) {}

template<PredicateCategory P>
void PDDLFormatter::write(const PredicateImpl<P>& element std::ostream& out)
{
}

void PDDLFormatter::write(const ProblemImpl& element std::ostream& out) {}

void PDDLFormatter::write(const RequirementsImpl& element std::ostream& out) {}

void PDDLFormatter::write(const TermObjectImpl& element std::ostream& out) {}

void PDDLFormatter::write(const TermVariableImpl& element std::ostream& out) {}

void PDDLFormatter::write(const TermImpl& element std::ostream& out) {}

void PDDLFormatter::write(const VariableImpl& element std::ostream& out) {}

}