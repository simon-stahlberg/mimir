#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_STATISTICS_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir
{

class GroundedAAGStatistics
{
private:
    uint64_t m_num_delete_free_reachable_ground_atoms;
    uint64_t m_num_delete_free_actions;
    uint64_t m_num_delete_free_axioms;

    uint64_t m_num_ground_actions;
    uint64_t m_num_nodes_in_action_match_tree;

    uint64_t m_num_ground_axioms;
    uint64_t m_num_nodes_in_axiom_match_tree;

public:
    GroundedAAGStatistics() :
        m_num_delete_free_reachable_ground_atoms(0),
        m_num_delete_free_actions(0),
        m_num_delete_free_axioms(0),
        m_num_ground_actions(0),
        m_num_nodes_in_action_match_tree(0),
        m_num_ground_axioms(0),
        m_num_nodes_in_axiom_match_tree(0)
    {
    }

    void set_num_delete_free_reachable_ground_atoms(uint64_t value) { m_num_delete_free_reachable_ground_atoms = value; }
    void set_num_delete_free_actions(uint64_t value) { m_num_delete_free_actions = value; }
    void set_num_delete_free_axioms(uint64_t value) { m_num_delete_free_axioms = value; }

    void set_num_ground_actions(uint64_t value) { m_num_ground_actions = value; }
    void set_num_nodes_in_action_match_tree(uint64_t value) { m_num_nodes_in_action_match_tree = value; }

    void set_num_ground_axioms(uint64_t value) { m_num_ground_axioms = value; }
    void set_num_nodes_in_axiom_match_tree(uint64_t value) { m_num_nodes_in_axiom_match_tree = value; }

    uint64_t get_num_delete_free_reachable_ground_atoms() const { return m_num_delete_free_reachable_ground_atoms; }
    uint64_t get_num_delete_free_actions() const { return m_num_delete_free_actions; }
    uint64_t get_num_delete_free_axioms() const { return m_num_delete_free_axioms; }

    uint64_t get_num_ground_actions() const { return m_num_ground_actions; }
    uint64_t get_num_nodes_in_action_match_tree() const { return m_num_nodes_in_action_match_tree; }

    uint64_t get_num_ground_axioms() const { return m_num_ground_axioms; }
    uint64_t get_num_nodes_in_axiom_match_tree() const { return m_num_nodes_in_axiom_match_tree; }
};

}

#endif
