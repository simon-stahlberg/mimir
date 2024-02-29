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

#ifndef MIMIR_FORMALISM_PROBLEM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_PROBLEM_DECLARATIONS_HPP_

#include "../domain/declarations.hpp"

#include <unordered_map>
#include <vector>

namespace mimir
{
class GroundAtomImpl;
using GroundAtom = const GroundAtomImpl*;
using GroundAtomList = std::vector<GroundAtom>;

class GroundLiteralImpl;
using GroundLiteral = const GroundLiteralImpl*;
using GroundLiteralList = std::vector<GroundLiteral>;

class NumericFluentImpl;
using NumericFluent = const NumericFluentImpl*;
using NumericFluentList = std::vector<NumericFluent>;

class OptimizationMetricImpl;
using OptimizationMetric = const OptimizationMetricImpl*;

class ProblemImpl;
using Problem = const ProblemImpl*;
using ProblemList = std::vector<Problem>;
}

#endif
