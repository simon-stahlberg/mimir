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

#ifndef MIMIR_FORMALISM_METRIC_HPP_
#define MIMIR_FORMALISM_METRIC_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/problem/pddl/metric.hpp>
#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class OptimizationMetricImpl : public loki::Base<OptimizationMetricImpl>
{
private:
    loki::pddl::OptimizationMetricEnum m_optimization_metric;
    FunctionExpression m_function_expression;

    // Below: add additional members if needed and initialize them in the constructor

    OptimizationMetricImpl(int identifier, loki::pddl::OptimizationMetricEnum optimization_metric, FunctionExpression function_expression);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const OptimizationMetricImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<OptimizationMetricImpl>;

public:
    loki::pddl::OptimizationMetricEnum get_optimization_metric() const;
    const FunctionExpression& get_function_expression() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::OptimizationMetric>
{
    bool operator()(const mimir::OptimizationMetric& left_metric, const mimir::OptimizationMetric& right_metric) const;
};

template<>
struct hash<mimir::OptimizationMetricImpl>
{
    std::size_t operator()(const mimir::OptimizationMetricImpl& metric) const;
};
}

#endif
