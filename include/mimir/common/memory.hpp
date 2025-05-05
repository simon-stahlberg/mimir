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
#include <fstream>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif
#if defined(__APPLE__)
#include <mach/mach.h>
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

inline int64_t get_peak_memory_usage_in_bytes()
{
    // On error, produces a warning on cerr and returns -1.
    int64_t memory_in_kb = -1;

#if defined(__APPLE__)
    // Based on http://stackoverflow.com/questions/63166
    task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (task_info(mach_task_self(), TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&t_info), &t_info_count) == KERN_SUCCESS)
    {
        memory_in_kb = t_info.virtual_size / 1024;
    }
#else
    std::ifstream procfile("/proc/self/status");
    std::string word;
    while (procfile >> word)
    {
        if (word == "VmPeak:")
        {
            procfile >> memory_in_kb;
            break;
        }
        // Skip to end of line.
        procfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    if (procfile.fail())
        memory_in_kb = -1;
#endif

    if (memory_in_kb == -1)
        std::cerr << "warning: could not determine peak memory" << std::endl;
    return memory_in_kb * 1024;
}

}

#endif
