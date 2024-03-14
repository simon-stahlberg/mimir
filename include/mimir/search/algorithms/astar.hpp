#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/event_handlers/interface.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/successor_state_generators/interface.hpp"

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
    std::unique_ptr<ISSG> m_state_repository;
    VState m_initial_state;
    std::unique_ptr<IAAG> m_successor_generator;
    std::unique_ptr<IHeuristic> m_heuristic;
    std::unique_ptr<IEventHandler> m_event_handler;

public:
    AStarAlgorithm(const Problem& problem,
                   PDDLFactories& pddl_factories,
                   std::unique_ptr<ISSG>&& state_repository,
                   std::unique_ptr<IAAG>&& successor_generator,
                   std::unique_ptr<IHeuristic>&& heuristic,
                   std::unique_ptr<IEventHandler>&& event_handler) :
        m_problem(problem),
        m_state_repository(std::move(state_repository)),
        m_initial_state(m_state_repository->get_or_create_initial_state(problem)),
        m_successor_generator(std::move(successor_generator)),
        m_heuristic(std::move(heuristic)),
        m_event_handler(std::move(event_handler))
    {
    }

    SearchStatus find_solution(VActionList& out_plan) override
    {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }
};

}

#endif
