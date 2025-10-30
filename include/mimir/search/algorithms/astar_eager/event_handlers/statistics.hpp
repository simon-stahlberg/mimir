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

#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_EAGER_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_EAGER_EVENT_HANDLERS_STATISTICS_HPP_

#include "mimir/common/declarations.hpp"

#include <chrono>
#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

namespace mimir::search::astar_eager
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

    std::map<double, uint64_t> m_num_generated_until_f_value;
    std::map<double, uint64_t> m_num_expanded_until_f_value;
    std::map<double, uint64_t> m_num_deadends_until_f_value;
    std::map<double, uint64_t> m_num_pruned_until_f_value;

    uint64_t m_num_reached_fluent_atoms;
    uint64_t m_num_reached_derived_atoms;

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
        m_num_generated_until_f_value(),
        m_num_expanded_until_f_value(),
        m_num_deadends_until_f_value(),
        m_num_pruned_until_f_value(),
        m_num_reached_fluent_atoms(0),
        m_num_reached_derived_atoms(0),
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
    void on_finish_f_layer(ContinuousCost f_value)
    {
        m_num_generated_until_f_value.emplace(f_value, m_num_generated);
        m_num_expanded_until_f_value.emplace(f_value, m_num_expanded);
        m_num_deadends_until_f_value.emplace(f_value, m_num_deadends);
        m_num_pruned_until_f_value.emplace(f_value, m_num_pruned);
    }

    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }
    void increment_num_pruned() { ++m_num_pruned; }
    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    void set_num_reached_fluent_atoms(uint64_t num_reached_fluent_atoms) { m_num_reached_fluent_atoms = num_reached_fluent_atoms; }
    void set_num_reached_derived_atoms(uint64_t num_reached_derived_atoms) { m_num_reached_derived_atoms = num_reached_derived_atoms; }

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
    std::chrono::milliseconds get_current_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_search_start_time_point);
    }

    uint64_t get_num_reached_fluent_atoms() const { return m_num_reached_fluent_atoms; }
    uint64_t get_num_reached_derived_atoms() const { return m_num_reached_derived_atoms; }
    uint64_t get_num_states() const { return m_num_states; }
    uint64_t get_num_nodes() const { return m_num_nodes; }
    uint64_t get_num_actions() const { return m_num_actions; }
    uint64_t get_num_axioms() const { return m_num_axioms; }

    const std::map<double, uint64_t>& get_num_generated_until_f_value() const { return m_num_generated_until_f_value; }
    const std::map<double, uint64_t>& get_num_expanded_until_f_value() const { return m_num_expanded_until_f_value; }
    const std::map<double, uint64_t>& get_num_deadends_until_f_value() const { return m_num_deadends_until_f_value; }
    const std::map<double, uint64_t>& get_num_pruned_until_f_value() const { return m_num_pruned_until_f_value; }
};

/**
 * Types
 */

using StatisticsList = std::vector<Statistics>;

}

#endif
