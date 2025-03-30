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

#ifndef MIMIR_COMMON_TYPES_HPP_
#define MIMIR_COMMON_TYPES_HPP_

#include "mimir/common/concepts.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <loki/loki.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir
{
using Count = uint32_t;

using Index = uint32_t;
using IndexList = std::vector<Index>;
using IndexSet = std::unordered_set<Index>;
template<size_t K>
using IndexArray = std::array<Index, K>;
template<typename T>
using IndexMap = std::unordered_map<Index, T>;
template<typename Key>
using ToIndexMap = std::unordered_map<Key, Index, loki::Hash<Key>, loki::EqualTo<Key>>;

template<typename Key, typename Value>
using KeyValueMap = std::unordered_map<Key, Value, loki::Hash<Key>, loki::EqualTo<Key>>;

template<typename Value>
using ValueSet = std::unordered_set<Value, loki::Hash<Value>, loki::EqualTo<Value>>;

static const Index MAX_INDEX = std::numeric_limits<Index>::max();

using ContinuousCost = double;
using ContinuousCostList = std::vector<ContinuousCost>;
using ContinuousCostMatrix = std::vector<ContinuousCostList>;
using DiscreteCost = int32_t;
using DiscreteCostList = std::vector<DiscreteCost>;
using DiscreteCostMatrix = std::vector<ContinuousCostList>;

static const DiscreteCost UNDEFINED_DISCRETE_COST = std::numeric_limits<DiscreteCost>::max();
static const ContinuousCost UNDEFINED_CONTINUOUS_COST = std::numeric_limits<ContinuousCost>::max();

}

#endif
