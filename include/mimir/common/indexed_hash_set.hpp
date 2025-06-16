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

#ifndef MIMIR_COMMON_INDEXED_HASH_SET_HPP_
#define MIMIR_COMMON_INDEXED_HASH_SET_HPP_

#include "mimir/common/segmented_vector.hpp"
#include "mimir/common/types.hpp"

#include <absl/container/flat_hash_map.h>
#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir
{
template<typename T, typename Hash = loki::Hash<loki::ObserverPtr<const T>>, typename EqualTo = loki::EqualTo<loki::ObserverPtr<const T>>>
class IndexedHashSet
{
private:
    SegmentedVector<T> m_vec;

    absl::flat_hash_map<loki::ObserverPtr<const T>, Index, Hash, EqualTo> m_map;

public:
    IndexedHashSet() : m_vec(), m_map() {}

    auto insert(const T& value)
    {
        m_vec.push_back(value);

        auto result = m_map.emplace(loki::ObserverPtr<const T>(&m_vec.back()), m_map.size());

        if (!result.second)
        {
            m_vec.pop_back();
        }

        return result;
    }

    auto insert(T&& value)
    {
        m_vec.push_back(std::move(value));

        auto result = m_map.emplace(loki::ObserverPtr<const T>(&m_vec.back()), m_map.size());

        if (!result.second)
        {
            m_vec.pop_back();
        }

        return result;
    }

    bool contains(const T& value) const { return m_map.contains(loki::ObserverPtr<const T>(&value)); }

    auto find(const T& value) const { return m_map.find(loki::ObserverPtr<const T>(&value)); }

    auto begin() const { return m_map.begin(); }
    auto end() const { return m_map.end(); }

    Index at(const T& value) const { return m_map.at(loki::ObserverPtr<const T>(&value)); }

    const T& operator[](size_t pos) const { return m_vec[pos]; }

    size_t size() const { return m_vec.size(); }
};
}

#endif
