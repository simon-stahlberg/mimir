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

#include "mimir/common/arithmetics.hpp"

#include <chrono>
#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir::search::brfs
{

class Statistics
{
private:
    uint64_t m_num_generated;
    uint64_t m_num_expanded;
    uint64_t m_num_deadends;
    uint64_t m_num_pruned;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

    std::vector<uint64_t> m_num_generated_until_g_value;
    std::vector<uint64_t> m_num_expanded_until_g_value;
    std::vector<uint64_t> m_num_deadends_until_g_value;
    std::vector<uint64_t> m_num_pruned_until_g_value;

    uint64_t m_num_reached_fluent_atoms;
    uint64_t m_num_reached_derived_atoms;

    uint64_t m_num_bytes_for_unextended_state_portion;
    uint64_t m_num_bytes_for_extended_state_portion;
    uint64_t m_num_bytes_for_nodes;
    uint64_t m_num_bytes_for_actions;
    uint64_t m_num_bytes_for_axioms;

    uint64_t m_num_states;
    uint64_t m_num_nodes;
    uint64_t m_num_actions;
    uint64_t m_num_axioms;

public:
    Statistics() :
        m_num_generated(0),
        m_num_expanded(0),
        m_num_deadends(0),
        m_num_pruned(0),
        m_num_generated_until_g_value(),
        m_num_expanded_until_g_value(),
        m_num_deadends_until_g_value(),
        m_num_pruned_until_g_value(),
        m_num_reached_fluent_atoms(0),
        m_num_reached_derived_atoms(0),
        m_num_bytes_for_unextended_state_portion(0),
        m_num_bytes_for_extended_state_portion(0),
        m_num_bytes_for_nodes(0),
        m_num_bytes_for_actions(0),
        m_num_bytes_for_axioms(0),
        m_num_states(0),
        m_num_nodes(0),
        m_num_actions(0),
        m_num_axioms(0)
    {
    }

    /**
     * Setters
     */

    /// @brief Store information for the layer
    void on_finish_g_layer()
    {
        m_num_generated_until_g_value.push_back(m_num_generated);
        m_num_expanded_until_g_value.push_back(m_num_expanded);
        m_num_deadends_until_g_value.push_back(m_num_deadends);
        m_num_pruned_until_g_value.push_back(m_num_pruned);
    }

    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }
    void increment_num_pruned() { ++m_num_pruned; }
    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    void set_num_reached_fluent_atoms(uint64_t num_reached_fluent_atoms) { m_num_reached_fluent_atoms = num_reached_fluent_atoms; }
    void set_num_reached_derived_atoms(uint64_t num_reached_derived_atoms) { m_num_reached_derived_atoms = num_reached_derived_atoms; }

    void set_num_bytes_for_unextended_state_portion(uint64_t num_bytes_for_unextended_state_portion)
    {
        m_num_bytes_for_unextended_state_portion = num_bytes_for_unextended_state_portion;
    }

    void set_num_bytes_for_extended_state_portion(uint64_t num_bytes_for_extended_state_portion)
    {
        m_num_bytes_for_extended_state_portion = num_bytes_for_extended_state_portion;
    }
    void set_num_bytes_for_nodes(uint64_t num_bytes_for_nodes) { m_num_bytes_for_nodes = num_bytes_for_nodes; }
    void set_num_bytes_for_actions(uint64_t num_bytes_for_actions) { m_num_bytes_for_actions = num_bytes_for_actions; }
    void set_num_bytes_for_axioms(uint64_t num_bytes_for_axioms) { m_num_bytes_for_axioms = num_bytes_for_axioms; }
    void set_num_states(uint64_t num_states) { m_num_states = num_states; }
    void set_num_nodes(uint64_t num_nodes) { m_num_nodes = num_nodes; }
    void set_num_actions(uint64_t num_actions) { m_num_actions = num_actions; }
    void set_num_axioms(uint64_t num_axioms) { m_num_axioms = num_axioms; }

    /**
     * Getters
     */

    uint64_t get_num_generated() const { return m_num_generated; }
    uint64_t get_num_expanded() const { return m_num_expanded; }
    uint64_t get_num_deadends() const { return m_num_deadends; }
    uint64_t get_num_pruned() const { return m_num_pruned; }

    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }

    uint64_t get_num_reached_fluent_atoms() const { return m_num_reached_fluent_atoms; }
    uint64_t get_num_reached_derived_atoms() const { return m_num_reached_derived_atoms; }
    uint64_t get_num_bytes_for_unextended_state_portion() const { return m_num_bytes_for_unextended_state_portion; }
    uint64_t get_num_bytes_for_extended_state_portion() const { return m_num_bytes_for_extended_state_portion; }
    uint64_t get_num_bytes_for_nodes() const { return m_num_bytes_for_nodes; }
    uint64_t get_num_bytes_for_actions() const { return m_num_bytes_for_actions; }
    uint64_t get_num_bytes_for_axioms() const { return m_num_bytes_for_axioms; }
    uint64_t get_num_states() const { return m_num_states; }
    uint64_t get_num_nodes() const { return m_num_nodes; }
    uint64_t get_num_actions() const { return m_num_actions; }
    uint64_t get_num_axioms() const { return m_num_axioms; }

    const std::vector<uint64_t>& get_num_generated_until_g_value() const { return m_num_generated_until_g_value; }
    const std::vector<uint64_t>& get_num_expanded_until_g_value() const { return m_num_expanded_until_g_value; }
    const std::vector<uint64_t>& get_num_deadends_until_g_value() const { return m_num_deadends_until_g_value; }
    const std::vector<uint64_t>& get_num_pruned_until_g_value() const { return m_num_pruned_until_g_value; }
};

/**
 * Types
 */

using StatisticsList = std::vector<Statistics>;

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const Statistics& statistics)
{
    os << "[BrFS] Search time: " << statistics.get_search_time_ms().count() << "ms" << "\n"
       << "[BrFS] Number of generated states: " << statistics.get_num_generated() << "\n"
       << "[BrFS] Number of expanded states: " << statistics.get_num_expanded() << "\n"
       << "[BrFS] Number of pruned states: " << statistics.get_num_pruned() << "\n"
       << "[BrFS] Number of generated states until last g-layer: "
       << (statistics.get_num_generated_until_g_value().empty() ? 0 : statistics.get_num_generated_until_g_value().back()) << "\n"
       << "[BrFS] Number of expanded states until last g-layer: "
       << (statistics.get_num_expanded_until_g_value().empty() ? 0 : statistics.get_num_expanded_until_g_value().back()) << "\n"
       << "[BrFS] Number of pruned states until last g-layer: "
       << (statistics.get_num_pruned_until_g_value().empty() ? 0 : statistics.get_num_pruned_until_g_value().back()) << "\n"
       << "[BrFS] Number of reached fluent atoms: " << statistics.get_num_reached_fluent_atoms() << "\n"
       << "[BrFS] Number of reached derived atoms: " << statistics.get_num_reached_derived_atoms() << "\n"
       << "[BrFS] Number of bytes for unextended state portions: " << statistics.get_num_bytes_for_unextended_state_portion() << "\n"
       << "[BrFS] Number of bytes per unextended state portion: "
       << divide_or_zero(statistics.get_num_bytes_for_unextended_state_portion(), statistics.get_num_states()) << "\n"
       << "[BrFS] Number of bytes for extended state portions: " << statistics.get_num_bytes_for_extended_state_portion() << "\n"
       << "[BrFS] Number of bytes per extended state portion: "
       << divide_or_zero(statistics.get_num_bytes_for_extended_state_portion(), statistics.get_num_states()) << "\n"
       << "[BrFS] Number of states: " << statistics.get_num_states() << "\n"
       << "[BrFS] Number of bytes for states: "
       << statistics.get_num_bytes_for_unextended_state_portion() + statistics.get_num_bytes_for_extended_state_portion() << "\n"
       << "[BrFS] Number of bytes per state: "
       << divide_or_zero((statistics.get_num_bytes_for_unextended_state_portion() + statistics.get_num_bytes_for_extended_state_portion()),
                         statistics.get_num_states())
       << "\n"
       << "[BrFS] Number of nodes: " << statistics.get_num_nodes() << "\n"
       << "[BrFS] Number of bytes for nodes: " << statistics.get_num_bytes_for_nodes() << "\n"
       << "[BrFS] Number of bytes per node: " << divide_or_zero(statistics.get_num_bytes_for_nodes(), statistics.get_num_nodes()) << "\n"
       << "[BrFS] Number of actions: " << statistics.get_num_actions() << "\n"
       << "[BrFS] Number of bytes for actions: " << statistics.get_num_bytes_for_actions() << "\n"
       << "[BrFS] Number of bytes per action: " << divide_or_zero(statistics.get_num_bytes_for_actions(), statistics.get_num_actions()) << "\n"
       << "[BrFS] Number of axioms: " << statistics.get_num_axioms() << "\n"
       << "[BrFS] Number of bytes for axioms: " << statistics.get_num_bytes_for_axioms() << "\n"
       << "[BrFS] Number of bytes per axiom: " << divide_or_zero(statistics.get_num_bytes_for_axioms(), statistics.get_num_axioms()) << "\n"
       << "[BrFS] Total number of bytes used: "
       << (statistics.get_num_bytes_for_unextended_state_portion() + statistics.get_num_bytes_for_extended_state_portion()
           + statistics.get_num_bytes_for_nodes() + statistics.get_num_bytes_for_actions() + statistics.get_num_bytes_for_axioms());

    return os;
}

}

#endif
