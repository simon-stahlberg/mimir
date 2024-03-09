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

#include <mimir/formalism/factories.hpp>
#include <mimir/formalism/parsers/conditions.hpp>
#include <mimir/formalism/parsers/domain.hpp>
#include <mimir/formalism/parsers/object.hpp>
#include <mimir/formalism/parsers/requirements.hpp>
#include <mimir/formalism/parsers/ground_literal.hpp>
#include <mimir/formalism/parsers/metric.hpp>
#include <mimir/formalism/parsers/numeric_fluent.hpp>
#include <mimir/formalism/parsers/problem.hpp>

namespace mimir
{
Problem parse(loki::pddl::Problem problem, PDDLFactories& factories)
{
    // Add constants to objects in problem.
    const auto constants = parse(problem->get_domain()->get_constants(), factories);
    auto objects = parse(problem->get_objects(), factories);
    objects.insert(objects.end(), constants.begin(), constants.end());

    return factories.problems.get_or_create<ProblemImpl>(parse(problem->get_domain(), factories),
                                                         problem->get_name(),
                                                         parse(problem->get_requirements(), factories),
                                                         objects,
                                                         parse(problem->get_initial_literals(), factories),
                                                         parse(problem->get_numeric_fluents(), factories),
                                                         parse(problem->get_goal_condition(), factories),
                                                         (problem->get_optimization_metric().has_value() ?
                                                              std::optional<OptimizationMetric>(parse(problem->get_optimization_metric().value(), factories)) :
                                                              std::nullopt));
}
}
