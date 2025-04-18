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

#ifndef MIMIR_COMMON_MEMORY_HPP_
#define MIMIR_COMMON_MEMORY_HPP_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

namespace mimir
{

template<typename T, typename Hash, typename Equal, typename Alloc>
inline size_t get_memory_usage_in_bytes(const absl::flat_hash_set<T, Hash, Equal, Alloc>& table)
{
    // Calculate memory for elements (keys and values)
    size_t element_memory = table.capacity() * sizeof(T);

    // Metadata (control bytes) - 1 byte per slot
    size_t metadata_memory = table.capacity();

    // Total memory usage
    return element_memory + metadata_memory;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
inline size_t get_memory_usage_in_bytes(const absl::flat_hash_map<Key, Value, Hash, Equal, Alloc>& table)
{
    // Calculate memory for elements (keys and values)
    size_t element_memory = table.capacity() * (sizeof(Key) + sizeof(Value));

    // Metadata (control bytes) - 1 byte per slot
    size_t metadata_memory = table.capacity();

    // Total memory usage
    return element_memory + metadata_memory;
}

inline uint64_t get_peak_memory_usage()
{
#if defined(__linux__) || defined(__APPLE__)
    struct rusage usage
    {
    };
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
#if defined(__APPLE__)
        // On macOS, ru_maxrss is in bytes
        return usage.ru_maxrss;
#else
        // On Linux, ru_maxrss is in kilobytes
        return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024;
#endif
    }
    return 0;
#endif
}

}

#endif
