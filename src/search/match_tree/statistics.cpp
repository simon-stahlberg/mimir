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

#include "mimir/search/match_tree/statistics.hpp"

#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"

namespace mimir::match_tree
{

MatchTreeStatistics::MatchTreeStatistics() : m_num_nodes(0), m_generator_distribution() {}

void MatchTreeStatistics::increment_num_nodes() { ++m_num_nodes; }
void MatchTreeStatistics::insert_into_generator_distribution(size_t num_elements) { m_generator_distribution.push_back(num_elements); }
void MatchTreeStatistics::set_is_imperfect(bool is_imperfect) { m_is_imperfect = is_imperfect; }

void MatchTreeStatistics::set_construction_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point)
{
    m_construction_start_time_point = time_point;
}
void MatchTreeStatistics::set_construction_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point)
{
    m_construction_end_time_point = time_point;
}

size_t MatchTreeStatistics::get_num_nodes() const { return m_num_nodes; }
std::vector<size_t> MatchTreeStatistics::get_generator_distribution() const { return m_generator_distribution; }
bool MatchTreeStatistics::is_imperfect() const { return m_is_imperfect; }

std::chrono::milliseconds MatchTreeStatistics::get_total_construction_time_ms() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_construction_end_time_point - m_construction_start_time_point);
}

std::ostream& operator<<(std::ostream& os, const MatchTreeStatistics& statistics)
{
    os << "[MatchTreeStatistics] Number of nodes: " << statistics.get_num_nodes() << "\n"
       << "[MatchTreeStatistics] Is imperfect?: " << statistics.is_imperfect() << "\n"
       << "[MatchTreeStatistics] Generators Gini score: " << compute_gini_score(statistics.get_generator_distribution()) << std::endl;

    return os;
}
}
