#ifndef MIMIR_SEARCH_PLAN_HPP_
#define MIMIR_SEARCH_PLAN_HPP_

#include "mimir/search/actions.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace mimir
{

class Plan
{
private:
    std::vector<std::string> m_actions;
    uint64_t m_cost;

public:
    Plan(std::vector<std::string> actions, uint64_t cost) : m_actions(std::move(actions)), m_cost(cost) {}

    const std::vector<std::string>& get_actions() const { return m_actions; }

    uint64_t get_cost() const { return m_cost; }
};

/// @brief Translates a ground action list to a plan
extern Plan to_plan(const GroundActionList& action_view_list);

/// @brief Write the plan to an ostream.
extern std::ostream& operator<<(std::ostream& os, const Plan& plan);

}

#endif
