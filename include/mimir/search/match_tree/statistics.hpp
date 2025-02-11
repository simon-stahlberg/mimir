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

#ifndef MIMIR_SEARCH_MATCH_TREE_STATISTICS_HPP_
#define MIMIR_SEARCH_MATCH_TREE_STATISTICS_HPP_

#include <chrono>
#include <map>
#include <ostream>
#include <vector>

namespace mimir::match_tree
{
class MatchTreeStatistics
{
private:
    size_t m_num_nodes;
    bool m_is_imperfect;
    std::vector<size_t> m_generator_distribution;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_construction_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_construction_end_time_point;

public:
    MatchTreeStatistics();

    /**
     * Setters
     */

    void increment_num_nodes();
    void insert_into_generator_distribution(size_t num_elements);
    void set_is_imperfect(bool is_imperfect);
    void set_construction_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point);
    void set_construction_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point);

    /**
     * Getters
     */

    size_t get_num_nodes() const;
    std::vector<size_t> get_generator_distribution() const;
    bool is_imperfect() const;
    std::chrono::milliseconds get_total_construction_time_ms() const;
};

extern std::ostream& operator<<(std::ostream& os, const MatchTreeStatistics& statistics);
}

#endif
