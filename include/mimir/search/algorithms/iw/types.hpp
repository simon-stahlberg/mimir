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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_TYPES_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_TYPES_HPP_

#include <unordered_set>
#include <vector>

namespace mimir
{
/**
 * Large k for IW(k) is generally infeasible.
 * Hence, we use a small constant values within feasible range
 * allowing us to use stack allocated arrays.
 */

const int MAX_ARITY = 6;

/**
 * Number of initial atoms in the DynamicNoveltyTable
 */

const int INITIAL_TABLE_ATOMS = 64;

/**
 * Type aliases for readability
 */

using AtomIndex = int;
using AtomIndexList = std::vector<AtomIndex>;

using TupleIndex = int;
using TupleIndexList = std::vector<TupleIndex>;
using TupleIndexSet = std::unordered_set<TupleIndex>;

}

#endif
