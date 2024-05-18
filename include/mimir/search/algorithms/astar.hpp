#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <vector>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class AStarAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IDynamicAAG> m_successor_generator;
    std::shared_ptr<IDynamicSSG> m_state_repository;
    State m_initial_state;
    std::shared_ptr<IDynamicHeuristic> m_heuristic;
    std::shared_ptr<IEventHandler> m_event_handler;

public:
    /// @brief Simplest construction
    AStarAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator, std::shared_ptr<IDynamicHeuristic> heuristic) :
        AStarAlgorithm(applicable_action_generator,
                       std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                       std::move(heuristic),
                       std::make_shared<MinimalEventHandler>())
    {
    }

    /// @brief Complete construction
    AStarAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator,
                   std::shared_ptr<IDynamicSSG> successor_state_generator,
                   std::shared_ptr<IDynamicHeuristic> heuristic,
                   std::shared_ptr<IEventHandler> event_handler) :
        m_successor_generator(std::move(applicable_action_generator)),
        m_state_repository(std::move(successor_state_generator)),
        m_initial_state(m_state_repository->get_or_create_initial_state()),
        m_heuristic(std::move(heuristic)),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(std::vector<GroundAction>& out_plan) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }
};

}

#endif
