#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/planners/interface.hpp"

#include <tuple>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class SinglePlanner : public IPlanner
{
private:
    std::shared_ptr<IAlgorithm> m_algorithm;

public:
    SinglePlanner(std::shared_ptr<IAlgorithm> algorithm) : m_algorithm(std::move(algorithm)) {}

    std::tuple<SearchStatus, Plan> find_solution() override
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }
};

}

#endif
