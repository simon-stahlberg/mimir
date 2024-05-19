#ifndef MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_STATISTICS_HPP_

#include <chrono>
#include <cstdint>

namespace mimir
{

class AlgorithmStatistics
{
private:
    uint64_t m_g_value;
    uint64_t m_num_states_until_g_value;
    uint64_t m_num_generated;
    uint64_t m_num_expanded;
    uint64_t m_num_deadends;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

public:
    AlgorithmStatistics() : m_g_value(0), m_num_states_until_g_value(0), m_num_generated(0), m_num_expanded(0), m_num_deadends(0) {}

    void set_g_value(size_t g_value) { m_g_value = g_value; }
    void set_num_states_until_g_value(size_t num_states_until_g_value) { m_num_states_until_g_value = num_states_until_g_value; }
    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }
    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    uint64_t get_g_value() const { return m_g_value; }
    uint64_t get_num_states_until_g_value() const { return m_num_states_until_g_value; }
    uint64_t get_num_generated() const { return m_num_generated; }
    uint64_t get_num_expanded() const { return m_num_expanded; }
    uint64_t get_num_deadends() const { return m_num_deadends; }
    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const AlgorithmStatistics& statistics)
{
    os << "[AlgorithmStatistics] Search time: " << statistics.get_search_time_ms().count() << "ms"
       << "\n"
       << "[AlgorithmStatistics] Num expanded states: " << statistics.get_num_expanded() << "\n"
       << "[AlgorithmStatistics] Num generated states: " << statistics.get_num_generated();

    return os;
}

}

#endif
