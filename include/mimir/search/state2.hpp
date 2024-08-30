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

#ifndef MIMIR_SEARCH_STATE2_HPP_
#define MIMIR_SEARCH_STATE2_HPP_

#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/tuple.h"
#include "cista/serialization.h"
#include "cista/storage/unordered_set.h"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/flat_types.hpp"

#include <ostream>
#include <tuple>
#include <unordered_map>

namespace mimir
{
using StateImpl = cista::tuple<StateIndex, cista::offset::dynamic_bitset<uint64_t>, cista::offset::dynamic_bitset<uint64_t>>;
using State = const StateImpl*;

}

// Only hash/compare the non-extended portion of a state, and the problem.
// The extended portion is always equal for the same non-extended portion.
template<>
struct cista::storage::DerefStdHasher<mimir::StateImpl>
{
    size_t operator()(mimir::State ptr) const { return mimir::hash_combine(cista::get<1>(*ptr)); }
};

template<>
struct cista::storage::DerefStdEqualTo<mimir::StateImpl>
{
    size_t operator()(mimir::State lhs, mimir::State rhs) const { return cista::get<1>(*lhs) == cista::get<1>(*rhs); }
};

template<>
struct std::hash<mimir::State>
{
    size_t operator()(mimir::State element) const;
};

template<>
struct std::equal_to<mimir::State>
{
    bool operator()(mimir::State lhs, mimir::State rhs) const;
};

namespace mimir
{

using StateList = std::vector<State>;

// template<typename T>
// using StateMap = std::unordered_map<State, T>;
// using StateSet = std::unordered_set<State>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLFactories&>& data);
}

#endif
