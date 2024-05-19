#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_STATISTICS_HPP_

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
    uint64_t m_num_inapplicable_grounded_actions;

    uint64_t m_num_ground_axiom_cache_hits;
    uint64_t m_num_ground_axiom_cache_misses;
    uint64_t m_num_inapplicable_grounded_axioms;

    std::vector<uint64_t> m_num_ground_action_cache_hits_until_f_value;
    std::vector<uint64_t> m_num_ground_action_cache_misses_until_f_value;
    std::vector<uint64_t> m_num_inapplicable_grounded_actions_until_f_value;

    std::vector<uint64_t> m_num_ground_axiom_cache_hits_until_f_value;
    std::vector<uint64_t> m_num_ground_axiom_cache_misses_until_f_value;
    std::vector<uint64_t> m_num_inapplicable_grounded_axioms_until_f_value;

public:
    LiftedAAGStatistics() :
        m_num_ground_action_cache_hits(0),
        m_num_ground_action_cache_misses(0),
        m_num_inapplicable_grounded_actions(0),
        m_num_ground_axiom_cache_hits(0),
        m_num_ground_axiom_cache_misses(0),
        m_num_inapplicable_grounded_axioms(0),
        m_num_ground_action_cache_hits_until_f_value(),
        m_num_ground_action_cache_misses_until_f_value(),
        m_num_inapplicable_grounded_actions_until_f_value(),
        m_num_ground_axiom_cache_hits_until_f_value(),
        m_num_ground_axiom_cache_misses_until_f_value(),
        m_num_inapplicable_grounded_axioms_until_f_value()
    {
    }

    /// @brief Store information for the layer
    void on_finish_f_layer()
    {
        m_num_ground_action_cache_hits_until_f_value.push_back(m_num_ground_action_cache_hits);
        m_num_ground_action_cache_misses_until_f_value.push_back(m_num_ground_action_cache_misses);
        m_num_inapplicable_grounded_actions_until_f_value.push_back(m_num_inapplicable_grounded_actions);

        m_num_ground_axiom_cache_hits_until_f_value.push_back(m_num_ground_axiom_cache_hits);
        m_num_ground_axiom_cache_misses_until_f_value.push_back(m_num_ground_axiom_cache_misses);
        m_num_inapplicable_grounded_axioms_until_f_value.push_back(m_num_inapplicable_grounded_axioms);
    }

    void increment_num_ground_action_cache_hits() { ++m_num_ground_action_cache_hits; }
    void increment_num_ground_action_cache_misses() { ++m_num_ground_action_cache_misses; }
    void increment_num_inapplicable_grounded_actions() { ++m_num_inapplicable_grounded_actions; }

    void increment_num_ground_axiom_cache_hits() { ++m_num_ground_axiom_cache_hits; }
    void increment_num_ground_axiom_cache_misses() { ++m_num_ground_axiom_cache_misses; }
    void increment_num_inapplicable_grounded_axioms() { ++m_num_inapplicable_grounded_axioms; }

    uint64_t get_num_ground_action_cache_hits() const { return m_num_ground_action_cache_hits; }
    uint64_t get_num_ground_action_cache_misses() const { return m_num_ground_action_cache_misses; }
    uint64_t get_num_inapplicable_grounded_actions() const { return m_num_inapplicable_grounded_actions; }

    uint64_t get_num_ground_axiom_cache_hits() const { return m_num_ground_axiom_cache_hits; }
    uint64_t get_num_ground_axiom_cache_misses() const { return m_num_ground_axiom_cache_misses; }
    uint64_t get_num_inapplicable_grounded_axioms() const { return m_num_inapplicable_grounded_axioms; }

    const std::vector<uint64_t>& get_num_ground_action_cache_hits_until_f_value() const { return m_num_ground_action_cache_hits_until_f_value; }
    const std::vector<uint64_t>& get_num_ground_action_cache_misses_until_f_value() const { return m_num_ground_action_cache_misses_until_f_value; }
    const std::vector<uint64_t>& get_num_inapplicable_grounded_actions_until_f_value() const { return m_num_inapplicable_grounded_actions_until_f_value; }

    const std::vector<uint64_t>& get_num_ground_axiom_cache_hits_until_f_value() const { return m_num_ground_axiom_cache_hits_until_f_value; }
    const std::vector<uint64_t>& get_num_ground_axiom_cache_misses_until_f_value() const { return m_num_ground_axiom_cache_misses_until_f_value; }
    const std::vector<uint64_t>& get_num_inapplicable_grounded_axioms_until_f_value() const { return m_num_inapplicable_grounded_axioms_until_f_value; }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const LiftedAAGStatistics& statistics)
{
    os << "[LiftedAAGStatistics] Number of grounded action cache hits: " << statistics.get_num_ground_action_cache_hits() << "\n"
       << "[LiftedAAGStatistics] Number of grounded action cache hits until last f-layer: "
       << (statistics.get_num_ground_action_cache_hits_until_f_value().empty() ? 0 : statistics.get_num_ground_action_cache_hits_until_f_value().back()) << "\n"
       << "[LiftedAAGStatistics] Number of grounded action cache misses: " << statistics.get_num_ground_action_cache_misses() << "\n"
       << "[LiftedAAGStatistics] Number of grounded action cache misses until last f-layer: "
       << (statistics.get_num_ground_action_cache_misses_until_f_value().empty() ? 0 : statistics.get_num_ground_action_cache_misses_until_f_value().back())
       << "\n"
       << "[LiftedAAGStatistics] Number of generated inapplicable grounded actions: " << statistics.get_num_inapplicable_grounded_actions() << "\n"
       << "[LiftedAAGStatistics] Number of generated inapplicable grounded actions until last f-layer: "
       << (statistics.get_num_inapplicable_grounded_actions_until_f_value().empty() ? 0 :
                                                                                      statistics.get_num_inapplicable_grounded_actions_until_f_value().back())
       << "\n"
       << "[LiftedAAGStatistics] Number of grounded axiom cache hits: " << statistics.get_num_ground_axiom_cache_hits() << "\n"
       << "[LiftedAAGStatistics] Number of grounded axiom cache hits until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_hits_until_f_value().empty() ? 0 : statistics.get_num_ground_axiom_cache_hits_until_f_value().back()) << "\n"
       << "[LiftedAAGStatistics] Number of grounded axiom cache misses: " << statistics.get_num_ground_axiom_cache_misses() << "\n"
       << "[LiftedAAGStatistics] Number of grounded axiom cache misses until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_misses_until_f_value().empty() ? 0 : statistics.get_num_ground_axiom_cache_misses_until_f_value().back())
       << "\n"
       << "[LiftedAAGStatistics] Number of generated inapplicable grounded axioms: " << statistics.get_num_inapplicable_grounded_axioms() << "\n"
       << "[LiftedAAGStatistics] Number of generated inapplicable grounded axioms until last f-layer: "
       << (statistics.get_num_inapplicable_grounded_axioms_until_f_value().empty() ? 0 :
                                                                                     statistics.get_num_inapplicable_grounded_axioms_until_f_value().back());

    return os;
}

}

#endif
