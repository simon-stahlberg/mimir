#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/event_handlers/interface.hpp"
#include "mimir/search/heuristics/interface.hpp"
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
    Problem m_problem;
    std::shared_ptr<IDynamicSSG> m_state_repository;
    ConstView<StateDispatcher<StateReprTag>> m_initial_state;
    std::shared_ptr<IDynamicAAG> m_successor_generator;
    std::shared_ptr<IDynamicHeuristic> m_heuristic;
    std::shared_ptr<IEventHandler> m_event_handler;

public:
    AStarAlgorithm(const Problem& problem,
                   PDDLFactories& pddl_factories,
                   std::shared_ptr<IDynamicSSG> state_repository,
                   std::shared_ptr<IDynamicAAG> successor_generator,
                   std::shared_ptr<IDynamicHeuristic> heuristic,
                   std::shared_ptr<IEventHandler> event_handler) :
        m_problem(problem),
        m_state_repository(std::move(state_repository)),
        m_initial_state(m_state_repository->get_or_create_initial_state(problem)),
        m_successor_generator(std::move(successor_generator)),
        m_heuristic(std::move(heuristic)),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(std::vector<ConstView<ActionDispatcher<StateReprTag>>>& out_plan) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }
};

}

#endif
