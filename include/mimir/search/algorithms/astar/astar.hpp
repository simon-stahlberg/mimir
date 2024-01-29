#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_ASTAR_HPP_

#include "../interface.hpp"

#include "../../heuristics.hpp"

#include <vector>


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P
       , IsHeuristicTag H
       , IsStateTag S = BitsetStateTag
       , IsAAGTag AG = DefaultAAGTag
       , IsSSGTag SG = DefaultSSGTag>
struct AStarTag : public AlgorithmTag { };


/**
 * Specialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S, IsAAGTag AG, IsSSGTag SG>
class Algorithm<AlgorithmDispatcher<AStarTag<P, H, S, AG, SG>>>
    : public IAlgorithm<Algorithm<AlgorithmDispatcher<AStarTag<P, H, S, AG, SG>>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<P, S>>;
    using ActionViewList = std::vector<ActionView>;

    Problem m_problem;
    SSG<SSGDispatcher<SG, P, S>> m_state_repository;
    StateView m_initial_state;
    AAG<AAGDispatcher<AG, P, S>> m_successor_generator;
    Heuristic<HeuristicDispatcher<H, P, S>> m_heuristic;

    SearchStatus find_solution_impl(ActionViewList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAlgorithm;

public:
    Algorithm(const Problem& problem)
        : m_problem(problem)
        , m_state_repository(SSG<SSGDispatcher<SG, P, S>>())
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem))
        , m_successor_generator(AAG<AAGDispatcher<AG, P, S>>())
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S, IsAAGTag AG, IsSSGTag SG>
struct TypeTraits<Algorithm<AlgorithmDispatcher<AStarTag<P, H, S, AG, SG>>>> {
    using PlanningModeTag = P;
    using HeuristicTag = H;
    using StateTag = S;
    using AAGTag = AG;
    using SSGTag = SG;

    using ActionView = View<ActionDispatcher<P, S>>;
    using ActionViewList = std::vector<ActionView>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_