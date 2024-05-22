#include "mimir/search/plan.hpp"

namespace mimir
{

Plan to_plan(const GroundActionList& action_view_list)
{
    std::vector<std::string> actions;
    auto cost = 0;
    for (const auto action : action_view_list)
    {
        std::stringstream ss;
        ss << action;
        actions.push_back(ss.str());
        cost += action.get_cost();
    }
    return Plan(std::move(actions), cost);
}

std::ostream& operator<<(std::ostream& os, const Plan& plan)
{
    for (const auto& action : plan.get_actions())
    {
        os << action << "\n";
    }
    os << "; cost = " << plan.get_cost();

    return os;
}

}
