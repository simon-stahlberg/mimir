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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_EVENT_HANDLERS_STATISTICS_HPP_

#include "mimir/search/algorithms/brfs/event_handlers/statistics.hpp"

#include <chrono>
#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir::search::iw
{

class Statistics
{
private:
    brfs::StatisticsList m_brfs_statistics_by_arity;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

public:
    Statistics() : m_brfs_statistics_by_arity() {}

    void push_back_algorithm_statistics(brfs::Statistics algorithm_statistics) { m_brfs_statistics_by_arity.push_back(std::move(algorithm_statistics)); }

    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    int get_effective_width() const { return m_brfs_statistics_by_arity.size() - 1; }

    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }

    const brfs::StatisticsList& get_brfs_statistics_by_arity() const { return m_brfs_statistics_by_arity; }
};

/**
 * Types
 */

using StatisticsList = std::vector<Statistics>;

}

#endif
