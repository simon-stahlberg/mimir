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

#include "mimir/search/metric.hpp"

#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/problem.hpp"

using namespace mimir::formalism;

namespace mimir::search
{
ContinuousCost compute_initial_state_metric_value(const ProblemImpl& problem)
{
    if (problem.get_auxiliary_function_value().has_value())
    {
        return problem.get_auxiliary_function_value().value()->get_number();
    }

    return problem.get_optimization_metric().has_value() ? evaluate(problem.get_optimization_metric().value()->get_function_expression(),
                                                                    problem.get_initial_function_to_value<StaticTag>(),
                                                                    problem.get_initial_function_to_value<FluentTag>()) :
                                                           0.;
}

}
