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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir
{

class LiftedAAGStatistics
{
private:
    uint64_t m_num_ground_action_cache_hits;
    uint64_t m_num_ground_action_cache_misses;

    uint64_t m_num_ground_axiom_cache_hits;
    uint64_t m_num_ground_axiom_cache_misses;

    std::vector<uint64_t> m_num_ground_action_cache_hits_until_f_value;
    std::vector<uint64_t> m_num_ground_action_cache_misses_until_f_value;
    std::vector<uint64_t> m_num_inapplicable_grounded_actions_until_f_value;

    std::vector<uint64_t> m_num_ground_axiom_cache_hits_until_f_value;
    std::vector<uint64_t> m_num_ground_axiom_cache_misses_until_f_value;

public:
    LiftedAAGStatistics() :
        m_num_ground_action_cache_hits(0),
        m_num_ground_action_cache_misses(0),
        m_num_ground_axiom_cache_hits(0),
        m_num_ground_axiom_cache_misses(0),
        m_num_ground_action_cache_hits_until_f_value(),
        m_num_ground_action_cache_misses_until_f_value(),
        m_num_inapplicable_grounded_actions_until_f_value(),
        m_num_ground_axiom_cache_hits_until_f_value(),
        m_num_ground_axiom_cache_misses_until_f_value()
    {
    }

    /// @brief Store information for the layer
    void on_finish_f_layer()
    {
        m_num_ground_action_cache_hits_until_f_value.push_back(m_num_ground_action_cache_hits);
        m_num_ground_action_cache_misses_until_f_value.push_back(m_num_ground_action_cache_misses);

        m_num_ground_axiom_cache_hits_until_f_value.push_back(m_num_ground_axiom_cache_hits);
        m_num_ground_axiom_cache_misses_until_f_value.push_back(m_num_ground_axiom_cache_misses);
    }

    void increment_num_ground_action_cache_hits() { ++m_num_ground_action_cache_hits; }
    void increment_num_ground_action_cache_misses() { ++m_num_ground_action_cache_misses; }

    void increment_num_ground_axiom_cache_hits() { ++m_num_ground_axiom_cache_hits; }
    void increment_num_ground_axiom_cache_misses() { ++m_num_ground_axiom_cache_misses; }

    uint64_t get_num_ground_action_cache_hits() const { return m_num_ground_action_cache_hits; }
    uint64_t get_num_ground_action_cache_misses() const { return m_num_ground_action_cache_misses; }

    uint64_t get_num_ground_axiom_cache_hits() const { return m_num_ground_axiom_cache_hits; }
    uint64_t get_num_ground_axiom_cache_misses() const { return m_num_ground_axiom_cache_misses; }

    const std::vector<uint64_t>& get_num_ground_action_cache_hits_until_f_value() const { return m_num_ground_action_cache_hits_until_f_value; }
    const std::vector<uint64_t>& get_num_ground_action_cache_misses_until_f_value() const { return m_num_ground_action_cache_misses_until_f_value; }
    const std::vector<uint64_t>& get_num_inapplicable_grounded_actions_until_f_value() const { return m_num_inapplicable_grounded_actions_until_f_value; }

    const std::vector<uint64_t>& get_num_ground_axiom_cache_hits_until_f_value() const { return m_num_ground_axiom_cache_hits_until_f_value; }
    const std::vector<uint64_t>& get_num_ground_axiom_cache_misses_until_f_value() const { return m_num_ground_axiom_cache_misses_until_f_value; }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const LiftedAAGStatistics& statistics)
{
    os << "[LiftedAAG] Number of grounded action cache hits: " << statistics.get_num_ground_action_cache_hits() << std::endl
       << "[LiftedAAG] Number of grounded action cache hits until last f-layer: "
       << (statistics.get_num_ground_action_cache_hits_until_f_value().empty() ? 0 : statistics.get_num_ground_action_cache_hits_until_f_value().back())
       << std::endl
       << "[LiftedAAG] Number of grounded action cache misses: " << statistics.get_num_ground_action_cache_misses() << std::endl
       << "[LiftedAAG] Number of grounded action cache misses until last f-layer: "
       << (statistics.get_num_ground_action_cache_misses_until_f_value().empty() ? 0 : statistics.get_num_ground_action_cache_misses_until_f_value().back())
       << std::endl
       << "[LiftedAAG] Number of generated inapplicable grounded actions until last f-layer: "
       << (statistics.get_num_inapplicable_grounded_actions_until_f_value().empty() ? 0 :
                                                                                      statistics.get_num_inapplicable_grounded_actions_until_f_value().back())
       << std::endl
       << "[LiftedAAG] Number of grounded axiom cache hits: " << statistics.get_num_ground_axiom_cache_hits() << std::endl
       << "[LiftedAAG] Number of grounded axiom cache hits until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_hits_until_f_value().empty() ? 0 : statistics.get_num_ground_axiom_cache_hits_until_f_value().back())
       << std::endl
       << "[LiftedAAG] Number of grounded axiom cache misses: " << statistics.get_num_ground_axiom_cache_misses() << std::endl
       << "[LiftedAAG] Number of grounded axiom cache misses until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_misses_until_f_value().empty() ? 0 : statistics.get_num_ground_axiom_cache_misses_until_f_value().back());

    return os;
}

}

#endif
