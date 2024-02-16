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
       , IsStateTag S = BitsetStateTag>
struct AStarTag : public AlgorithmTag { };


/**
 * Specialized implementation class.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S>
class Algorithm<AlgorithmDispatcher<AStarTag<P, H, S>>>
    : public IAlgorithm<Algorithm<AlgorithmDispatcher<AStarTag<P, H, S>>>> {
private:
    using ConstStateView = ConstView<StateDispatcher<S, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, S>>;
    using ConstActionViewList = std::vector<ConstActionView>;

    Problem m_problem;
    SSG<SSGDispatcher<P, S>> m_state_repository;
    ConstStateView m_initial_state;
    AAG<AAGDispatcher<P, S>> m_successor_generator;
    Heuristic<HeuristicDispatcher<H, P, S>> m_heuristic;

    /* Implement IAlgorithm interface. */
    template<typename>
    friend class IAlgorithm;

    SearchStatus find_solution_impl(ConstActionViewList& out_plan) {
        // TODO (Dominik): implement
        return SearchStatus::FAILED;
    }


public:
    Algorithm(const Problem& problem, PDDLFactories& pddl_factories)
        : m_problem(problem)
        , m_state_repository(SSG<SSGDispatcher<P, S>>(problem))
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem))
        , m_successor_generator(AAG<AAGDispatcher<P, S>>(problem, pddl_factories))
        , m_heuristic(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsHeuristicTag H, IsStateTag S>
struct TypeTraits<Algorithm<AlgorithmDispatcher<AStarTag<P, H, S>>>> {
    using PlanningModeTag = P;
    using HeuristicTag = H;
    using StateTag = S;

    using ConstActionView = ConstView<ActionDispatcher<P, S>>;
    using ConstActionViewList = std::vector<ConstActionView>;
};


} 

#endif  
