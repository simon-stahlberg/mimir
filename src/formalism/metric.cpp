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

#include "mimir/formalism/metric.hpp"

#include "mimir/formalism/ground_function_expressions.hpp"

#include <cassert>
#include <loki/loki.hpp>

using namespace std;

namespace mimir
{
OptimizationMetricImpl::OptimizationMetricImpl(int identifier, loki::OptimizationMetricEnum optimization_metric, GroundFunctionExpression function_expression) :
    Base(identifier),
    m_optimization_metric(optimization_metric),
    m_function_expression(std::move(function_expression))
{
}

bool OptimizationMetricImpl::is_structurally_equivalent_to_impl(const OptimizationMetricImpl& other) const
{
    if (this != &other)
    {
        return (m_optimization_metric == other.m_optimization_metric) && (m_function_expression == other.m_function_expression);
    }
    return true;
}

size_t OptimizationMetricImpl::hash_impl() const { return loki::hash_combine(m_optimization_metric, m_function_expression); }

void OptimizationMetricImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << to_string(m_optimization_metric) << " ";
    std::visit(loki::StringifyVisitor(out, options), *m_function_expression);
    out << ")";
}

loki::OptimizationMetricEnum OptimizationMetricImpl::get_optimization_metric() const { return m_optimization_metric; }

const GroundFunctionExpression& OptimizationMetricImpl::get_function_expression() const { return m_function_expression; }

}
