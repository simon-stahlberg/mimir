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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_COMMON_DECLARATIONS_HPP_
#define MIMIR_COMMON_DECLARATIONS_HPP_

#include <array>
#include <boost/hana.hpp>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <loki/loki.hpp>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Index
 */

using Index = uint32_t;
using IndexPair = std::pair<Index, Index>;
using IndexPairList = std::vector<IndexPair>;
using IndexList = std::vector<Index>;
using IndexSet = std::unordered_set<Index>;
template<size_t K>
using IndexArray = std::array<Index, K>;
template<typename T>
using IndexMap = std::unordered_map<Index, T>;
template<typename Key>
using ToIndexMap = std::unordered_map<Key, Index, loki::Hash<Key>, loki::EqualTo<Key>>;

static const Index MAX_INDEX = std::numeric_limits<Index>::max();

/**
 * ContinuousCost
 */

using ContinuousCost = double;
using ContinuousCostList = std::vector<ContinuousCost>;
using ContinuousCostMatrix = std::vector<ContinuousCostList>;
using ContinuousCostMap = std::unordered_map<Index, ContinuousCost>;

static constexpr const ContinuousCost UNDEFINED_CONTINUOUS_COST = std::numeric_limits<ContinuousCost>::quiet_NaN();
static constexpr const ContinuousCost INFINITY_CONTINUOUS_COST = std::numeric_limits<ContinuousCost>::infinity();

/**
 * DiscreteCost
 */

using DiscreteCost = int32_t;
using DiscreteCostList = std::vector<DiscreteCost>;
using DiscreteCostMatrix = std::vector<ContinuousCostList>;
using DiscreteCostMap = std::unordered_map<Index, DiscreteCost>;

static constexpr const DiscreteCost UNDEFINED_DISCRETE_COST = std::numeric_limits<DiscreteCost>::max();
static constexpr const DiscreteCost MAX_DISCRETE_COST = std::numeric_limits<DiscreteCost>::max();

/**
 * Containers
 */

template<typename Key, typename Value>
using UnorderedMap = std::unordered_map<Key, Value, loki::Hash<Key>, loki::EqualTo<Key>>;
template<typename Value>
using UnorderedSet = std::unordered_set<Value, loki::Hash<Value>, loki::EqualTo<Value>>;

template<typename T, typename... Ds>
using HanaContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, T>...>;

template<template<typename> typename T, typename... Ds>
using HanaMappedContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Ds>, T<Ds>>...>;

template<typename T>
struct dependent_false : std::false_type
{
};

template<typename T>
concept IsFloatingPoint = std::is_floating_point_v<T>;

template<typename T, typename Value>
concept IsRangeOver = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, Value>;

template<typename T, typename Value>
concept IsConvertibleRangeOver = std::ranges::range<T> && std::convertible_to<std::ranges::range_value_t<T>, Value>;

template<typename T>
concept IsBackInsertibleRange = std::ranges::forward_range<T> &&                       // Must be a forward range
                                std::default_initializable<std::remove_cvref_t<T>> &&  // Must be default-constructible
                                requires(std::remove_cvref_t<T> container, std::ranges::range_value_t<T> value) {
                                    std::back_inserter(container);           // Must support std::back_inserter
                                    *std::back_inserter(container) = value;  // Must support inserting values
                                };

template<typename T>
concept IsHanaMap = std::same_as<typename boost::hana::tag_of<T>::type, boost::hana::map_tag>;

}

#endif
