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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_STATISTICS_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir
{

class GroundedAAGStatistics
{
private:
    uint64_t m_num_delete_free_reachable_fluent_ground_atoms;
    uint64_t m_num_delete_free_reachable_derived_ground_atoms;
    uint64_t m_num_delete_free_actions;
    uint64_t m_num_delete_free_axioms;

    uint64_t m_num_ground_actions;
    uint64_t m_num_nodes_in_action_match_tree;

    uint64_t m_num_ground_axioms;
    uint64_t m_num_nodes_in_axiom_match_tree;

public:
    GroundedAAGStatistics() :
        m_num_delete_free_reachable_fluent_ground_atoms(0),
        m_num_delete_free_reachable_derived_ground_atoms(0),
        m_num_delete_free_actions(0),
        m_num_delete_free_axioms(0),
        m_num_ground_actions(0),
        m_num_nodes_in_action_match_tree(0),
        m_num_ground_axioms(0),
        m_num_nodes_in_axiom_match_tree(0)
    {
    }

    void set_num_delete_free_reachable_fluent_ground_atoms(uint64_t value) { m_num_delete_free_reachable_fluent_ground_atoms = value; }
    void set_num_delete_free_reachable_derived_ground_atoms(uint64_t value) { m_num_delete_free_reachable_derived_ground_atoms = value; }
    void set_num_delete_free_actions(uint64_t value) { m_num_delete_free_actions = value; }
    void set_num_delete_free_axioms(uint64_t value) { m_num_delete_free_axioms = value; }

    void set_num_ground_actions(uint64_t value) { m_num_ground_actions = value; }
    void set_num_nodes_in_action_match_tree(uint64_t value) { m_num_nodes_in_action_match_tree = value; }

    void set_num_ground_axioms(uint64_t value) { m_num_ground_axioms = value; }
    void set_num_nodes_in_axiom_match_tree(uint64_t value) { m_num_nodes_in_axiom_match_tree = value; }

    uint64_t get_num_delete_free_reachable_fluent_ground_atoms() const { return m_num_delete_free_reachable_fluent_ground_atoms; }
    uint64_t get_num_delete_free_reachable_derived_ground_atoms() const { return m_num_delete_free_reachable_derived_ground_atoms; }
    uint64_t get_num_delete_free_actions() const { return m_num_delete_free_actions; }
    uint64_t get_num_delete_free_axioms() const { return m_num_delete_free_axioms; }

    uint64_t get_num_ground_actions() const { return m_num_ground_actions; }
    uint64_t get_num_nodes_in_action_match_tree() const { return m_num_nodes_in_action_match_tree; }

    uint64_t get_num_ground_axioms() const { return m_num_ground_axioms; }
    uint64_t get_num_nodes_in_axiom_match_tree() const { return m_num_nodes_in_axiom_match_tree; }
};

}

#endif
