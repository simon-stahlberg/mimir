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

#ifndef MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_SIW_EVENT_HANDLERS_STATISTICS_HPP_

#include "mimir/search/algorithms/iw/event_handlers/statistics.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <numeric>
#include <ostream>
#include <vector>

namespace mimir::search::siw
{

class Statistics
{
private:
    iw::StatisticsList m_iw_algorithm_statistics_by_subproblem;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

public:
    Statistics() : m_iw_algorithm_statistics_by_subproblem() {}

    void push_back_algorithm_statistics(iw::Statistics iw_statistics) { m_iw_algorithm_statistics_by_subproblem.push_back(std::move(iw_statistics)); }

    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    int get_maximum_effective_width() const
    {
        if (m_iw_algorithm_statistics_by_subproblem.empty())
        {
            return -1;
        }
        return std::max_element(m_iw_algorithm_statistics_by_subproblem.begin(),
                                m_iw_algorithm_statistics_by_subproblem.end(),
                                [](const auto& statistics_left, const auto& statistics_right)
                                { return statistics_left.get_effective_width() < statistics_right.get_effective_width(); })
            ->get_effective_width();
    }

    double get_average_effective_width() const
    {
        if (m_iw_algorithm_statistics_by_subproblem.empty())
        {
            return -1;
        }

        int total_width = std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                                          m_iw_algorithm_statistics_by_subproblem.end(),
                                          0,
                                          [](int sum, const auto& item) { return sum + item.get_effective_width(); });

        return static_cast<double>(total_width) / m_iw_algorithm_statistics_by_subproblem.size();
    }

    int get_num_generated() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item) { return sum + item.get_brfs_statistics_by_arity().back().get_num_generated(); });
    }

    int get_num_expanded() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item) { return sum + item.get_brfs_statistics_by_arity().back().get_num_expanded(); });
    }

    int get_num_pruned() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item) { return sum + item.get_brfs_statistics_by_arity().back().get_num_pruned(); });
    }

    int get_num_generated_until_last_g_layer() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item)
                               {
                                   return sum
                                          + ((item.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().empty()) ?
                                                 0 :
                                                 item.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back());
                               });
    }

    int get_num_expanded_until_last_g_layer() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item)
                               {
                                   return sum
                                          + ((item.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().empty()) ?
                                                 0 :
                                                 item.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back());
                               });
    }

    int get_num_pruned_until_last_g_layer() const
    {
        return std::accumulate(m_iw_algorithm_statistics_by_subproblem.begin(),
                               m_iw_algorithm_statistics_by_subproblem.end(),
                               0,
                               [](int sum, const auto& item)
                               {
                                   return sum
                                          + ((item.get_brfs_statistics_by_arity().back().get_num_pruned_until_g_value().empty()) ?
                                                 0 :
                                                 item.get_brfs_statistics_by_arity().back().get_num_pruned_until_g_value().back());
                               });
    }

    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }

    const iw::StatisticsList& get_iw_statistics_by_subproblem() const { return m_iw_algorithm_statistics_by_subproblem; }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const Statistics& statistics)
{
    os << "[SIW] Search time: " << statistics.get_search_time_ms().count() << "ms" << "\n"
       << "[SIW] Maximum effective width: " << statistics.get_maximum_effective_width() << "\n"
       << "[SIW] Average effective width: " << statistics.get_average_effective_width() << "\n"
       << "[SIW] Number of generated states: " << statistics.get_num_generated() << "\n"
       << "[SIW] Number of expanded states: " << statistics.get_num_expanded() << "\n"
       << "[SIW] Number of pruned states: " << statistics.get_num_pruned() << "\n"
       << "[SIW] Number of generated states until last f-layer: " << statistics.get_num_generated_until_last_g_layer() << "\n"
       << "[SIW] Number of expanded states until last f-layer: " << statistics.get_num_expanded_until_last_g_layer() << "\n"
       << "[SIW] Number of pruned states until last f-layer: " << statistics.get_num_pruned_until_last_g_layer();

    return os;
}

}

#endif
