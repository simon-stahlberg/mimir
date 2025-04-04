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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir::search::axiom_evaluator::lifted
{
class Statistics
{
private:
    uint64_t m_num_ground_axiom_cache_hits;
    uint64_t m_num_ground_axiom_cache_misses;

    std::vector<uint64_t> m_num_ground_axiom_cache_hits_per_search_layer;
    std::vector<uint64_t> m_num_ground_axiom_cache_misses_per_search_layer;

public:
    Statistics() :
        m_num_ground_axiom_cache_hits(0),
        m_num_ground_axiom_cache_misses(0),
        m_num_ground_axiom_cache_hits_per_search_layer(),
        m_num_ground_axiom_cache_misses_per_search_layer()
    {
    }

    /// @brief Store information for the layer
    void on_finish_search_layer()
    {
        m_num_ground_axiom_cache_hits_per_search_layer.push_back(m_num_ground_axiom_cache_hits);
        m_num_ground_axiom_cache_misses_per_search_layer.push_back(m_num_ground_axiom_cache_misses);
    }

    void increment_num_ground_axiom_cache_hits() { ++m_num_ground_axiom_cache_hits; }
    void increment_num_ground_axiom_cache_misses() { ++m_num_ground_axiom_cache_misses; }

    uint64_t get_num_ground_axiom_cache_hits() const { return m_num_ground_axiom_cache_hits; }
    uint64_t get_num_ground_axiom_cache_misses() const { return m_num_ground_axiom_cache_misses; }

    const std::vector<uint64_t>& get_num_ground_axiom_cache_hits_per_search_layer() const { return m_num_ground_axiom_cache_hits_per_search_layer; }
    const std::vector<uint64_t>& get_num_ground_axiom_cache_misses_per_search_layer() const { return m_num_ground_axiom_cache_misses_per_search_layer; }
};

inline std::ostream& operator<<(std::ostream& os, const Statistics& statistics)
{
    os << "[LiftedAxiomEvaluator] Number of grounded axiom cache hits: " << statistics.get_num_ground_axiom_cache_hits() << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache hits until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_hits_per_search_layer().empty() ? 0 : statistics.get_num_ground_axiom_cache_hits_per_search_layer().back())
       << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache misses: " << statistics.get_num_ground_axiom_cache_misses() << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache misses until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_misses_per_search_layer().empty() ? 0 :
                                                                                     statistics.get_num_ground_axiom_cache_misses_per_search_layer().back());

    return os;
}

}

#endif
