#ifndef MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_EVENT_HANDLERS_STATISTICS_HPP_

#include <chrono>
#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir
{

class AlgorithmStatistics
{
private:
    uint64_t m_num_generated;
    uint64_t m_num_expanded;
    uint64_t m_num_deadends;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_start_time_point;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_search_end_time_point;

    std::vector<uint64_t> m_num_generated_until_f_value;
    std::vector<uint64_t> m_num_expanded_until_f_value;
    std::vector<uint64_t> m_num_deadends_until_f_value;

public:
    AlgorithmStatistics() :
        m_num_generated(0),
        m_num_expanded(0),
        m_num_deadends(0),
        m_num_generated_until_f_value(),
        m_num_expanded_until_f_value(),
        m_num_deadends_until_f_value()
    {
    }

    /// @brief Store information for the layer
    void on_finish_f_layer()
    {
        m_num_generated_until_f_value.push_back(m_num_generated);
        m_num_expanded_until_f_value.push_back(m_num_expanded);
        m_num_deadends_until_f_value.push_back(m_num_deadends);
    }

    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }
    void set_search_start_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_start_time_point = time_point; }
    void set_search_end_time_point(std::chrono::time_point<std::chrono::high_resolution_clock> time_point) { m_search_end_time_point = time_point; }

    uint64_t get_num_generated() const { return m_num_generated; }
    uint64_t get_num_expanded() const { return m_num_expanded; }
    uint64_t get_num_deadends() const { return m_num_deadends; }

    std::chrono::milliseconds get_search_time_ms() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_search_end_time_point - m_search_start_time_point);
    }

    const std::vector<uint64_t>& get_num_generated_until_f_value() const { return m_num_generated_until_f_value; }
    const std::vector<uint64_t>& get_num_expanded_until_f_value() const { return m_num_expanded_until_f_value; }
    const std::vector<uint64_t>& get_num_deadends_until_f_value() const { return m_num_deadends_until_f_value; }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const AlgorithmStatistics& statistics)
{
    os << "[AlgorithmStatistics] Search time: " << statistics.get_search_time_ms().count() << "ms"
       << "\n"
       << "[AlgorithmStatistics] Number of generated states: " << statistics.get_num_generated() << "\n"
       << "[AlgorithmStatistics] Number of expanded states: " << statistics.get_num_expanded() << "\n"
       << "[AlgorithmStatistics] Number of generated states until last f-layer: "
       << (statistics.get_num_generated_until_f_value().empty() ? 0 : statistics.get_num_generated_until_f_value().back()) << "\n"
       << "[AlgorithmStatistics] Number of expanded states until last f-layer: "
       << (statistics.get_num_expanded_until_f_value().empty() ? 0 : statistics.get_num_expanded_until_f_value().back());

    return os;
}

}

#endif