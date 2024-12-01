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

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <cstddef>
#include <ostream>

namespace mimir
{

class PDDLFormatter
{
private:
    // The indentation in the current level.
    size_t m_indent;
    // The amount of indentation added per nesting
    size_t m_add_indent;
    // Whether action costs are enabled.
    bool m_action_costs;

public:
    PDDLFormatter(size_t indent = 0, size_t add_indent = 4, bool action_costs = false);

    void write(const ActionImpl& element, std::ostream& out);
    template<PredicateTag P>
    void write(const AtomImpl<P>& element, std::ostream& out);
    void write(const AxiomImpl& element, std::ostream& out);
    void write(const DomainImpl& element, std::ostream& out);
    void write(const EffectStripsImpl& element, std::ostream& out);
    void write(const EffectConditionalImpl& element, std::ostream& out);
    void write(const FunctionExpressionNumberImpl& element, std::ostream& out);
    void write(const FunctionExpressionBinaryOperatorImpl& element, std::ostream& out);
    void write(const FunctionExpressionMultiOperatorImpl& element, std::ostream& out);
    void write(const FunctionExpressionMinusImpl& element, std::ostream& out);
    void write(const FunctionExpressionFunctionImpl& element, std::ostream& out);
    void write(const FunctionExpressionImpl& element, std::ostream& out);
    void write(const FunctionSkeletonImpl& element, std::ostream& out);
    void write(const FunctionImpl& element, std::ostream& out);
    template<PredicateTag P>
    void write(const GroundAtomImpl<P>& element, std::ostream& out);
    void write(const GroundFunctionExpressionNumberImpl& element, std::ostream& out);
    void write(const GroundFunctionExpressionBinaryOperatorImpl& element, std::ostream& out);
    void write(const GroundFunctionExpressionMultiOperatorImpl& element, std::ostream& out);
    void write(const GroundFunctionExpressionMinusImpl& element, std::ostream& out);
    void write(const GroundFunctionExpressionFunctionImpl& element, std::ostream& out);
    void write(const GroundFunctionExpressionImpl& element, std::ostream& out);
    void write(const GroundFunctionImpl& element, std::ostream& out);
    template<PredicateTag P>
    void write(const GroundLiteralImpl<P>& element, std::ostream& out);
    template<PredicateTag P>
    void write(const LiteralImpl<P>& element, std::ostream& out);
    void write(const OptimizationMetricImpl& element, std::ostream& out);
    void write(const NumericFluentImpl& element, std::ostream& out);
    void write(const ObjectImpl& element, std::ostream& out);
    template<PredicateTag P>
    void write(const PredicateImpl<P>& element, std::ostream& out);
    void write(const ProblemImpl& element, std::ostream& out);
    void write(const RequirementsImpl& element, std::ostream& out);
    void write(const TermImpl& element, std::ostream& out);
    void write(const VariableImpl& element, std::ostream& out);
};

}

#endif