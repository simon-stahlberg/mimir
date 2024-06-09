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

#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_EVENT_HANDLERS_STATISTICS_HPP_

#include <chrono>
#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir
{

class BrFSAlgorithmStatistics
{
private:
    uint64_t m_num_generated;
    uint64_t m_num_expanded;
    uint64_t m_num_deadends;
    uint64_t m_num_pruned;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

    std::vector<uint64_t> m_num_generated_until_f_value;
    std::vector<uint64_t> m_num_expanded_until_f_value;
    std::vector<uint64_t> m_num_deadends_until_f_value;
    std::vector<uint64_t> m_num_pruned_until_f_value;

public:
    BrFSAlgorithmStatistics() :
        m_num_generated(0),
        m_num_expanded(0),
        m_num_deadends(0),
        m_num_pruned(0),
        m_num_generated_until_f_value(),
        m_num_expanded_until_f_value(),
        m_num_deadends_until_f_value(),
        m_num_pruned_until_f_value()
    {
    }

    /// @brief Store information for the layer
    void on_finish_f_layer()
    {
        m_num_generated_until_f_value.push_back(m_num_generated);
        m_num_expanded_until_f_value.push_back(m_num_expanded);
        m_num_deadends_until_f_value.push_back(m_num_deadends);
        m_num_pruned_until_f_value.push_back(m_num_pruned);
    }

    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }
    void increment_num_pruned() { ++m_num_pruned; }
    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    uint64_t get_num_generated() const { return m_num_generated; }
    uint64_t get_num_expanded() const { return m_num_expanded; }
    uint64_t get_num_deadends() const { return m_num_deadends; }
    uint64_t get_num_pruned() const { return m_num_pruned; }

    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }

    const std::vector<uint64_t>& get_num_generated_until_f_value() const { return m_num_generated_until_f_value; }
    const std::vector<uint64_t>& get_num_expanded_until_f_value() const { return m_num_expanded_until_f_value; }
    const std::vector<uint64_t>& get_num_deadends_until_f_value() const { return m_num_deadends_until_f_value; }
    const std::vector<uint64_t>& get_num_pruned_until_f_value() const { return m_num_pruned_until_f_value; }
};

/**
 * Types
 */

using BrFSAlgorithmStatisticsList = std::vector<BrFSAlgorithmStatistics>;

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const BrFSAlgorithmStatistics& statistics)
{
    os << "[BrFS] Search time: " << statistics.get_search_time_ms().count() << "ms"
       << "\n"
       << "[BrFS] Number of generated states: " << statistics.get_num_generated() << "\n"
       << "[BrFS] Number of expanded states: " << statistics.get_num_expanded() << "\n"
       << "[BrFS] Number of pruned states: " << statistics.get_num_pruned() << "\n"
       << "[BrFS] Number of generated states until last f-layer: "
       << (statistics.get_num_generated_until_f_value().empty() ? 0 : statistics.get_num_generated_until_f_value().back()) << "\n"
       << "[BrFS] Number of expanded states until last f-layer: "
       << (statistics.get_num_expanded_until_f_value().empty() ? 0 : statistics.get_num_expanded_until_f_value().back()) << "\n"
       << "[BrFS] Number of pruned states until last f-layer: "
       << (statistics.get_num_pruned_until_f_value().empty() ? 0 : statistics.get_num_pruned_until_f_value().back());

    return os;
}

}

#endif
