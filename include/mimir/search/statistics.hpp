#ifndef MIMIR_SEARCH_STATISTICS_HPP_
#define MIMIR_SEARCH_STATISTICS_HPP_

#include <cstdint>


namespace mimir
{

class Statistics
{
private:
    uint64_t m_num_generated;
    uint64_t m_num_expanded;
    uint64_t m_num_deadends;

public:
    Statistics() :
        m_num_generated(0),
        m_num_expanded(0),
        m_num_deadends(0) {}

    void increment_num_generated() { ++m_num_generated; }
    void increment_num_expanded() { ++m_num_expanded; }
    void increment_num_deadends() { ++m_num_deadends; }

    uint64_t get_num_generated() const { return m_num_generated; }
    uint64_t get_num_expanded() const { return m_num_expanded; }
    uint64_t get_num_deadends() const { return m_num_deadends; }
};

}

#endif
