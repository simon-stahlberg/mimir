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

#ifndef MIMIR_FORMALISM_METRIC_HPP_
#define MIMIR_FORMALISM_METRIC_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class OptimizationMetricImpl
{
private:
    Index m_index;
    loki::OptimizationMetricEnum m_optimization_metric;
    GroundFunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    OptimizationMetricImpl(Index index, loki::OptimizationMetricEnum optimization_metric, GroundFunctionExpression function_expression);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    OptimizationMetricImpl(const OptimizationMetricImpl& other) = delete;
    OptimizationMetricImpl& operator=(const OptimizationMetricImpl& other) = delete;
    OptimizationMetricImpl(OptimizationMetricImpl&& other) = default;
    OptimizationMetricImpl& operator=(OptimizationMetricImpl&& other) = default;

    Index get_index() const;
    loki::OptimizationMetricEnum get_optimization_metric() const;
    GroundFunctionExpression get_function_expression() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_optimization_metric(), get_function_expression()); }
};

extern std::ostream& operator<<(std::ostream& out, const OptimizationMetricImpl& element);

extern std::ostream& operator<<(std::ostream& out, OptimizationMetric element);

}

#endif
