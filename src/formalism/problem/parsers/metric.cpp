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

#include <mimir/formalism/problem/parsers/metric.hpp>

#include <mimir/formalism/domain/parsers/function_expressions.hpp>


namespace mimir 
{
    OptimizationMetric parse(loki::pddl::OptimizationMetric optimization_metric, PDDLFactories& factories)
    {
        return factories.optimization_metrics.get_or_create<OptimizationMetricImpl>(
            optimization_metric->get_optimization_metric(),
            parse(optimization_metric->get_function_expression(), factories));
    }
}
