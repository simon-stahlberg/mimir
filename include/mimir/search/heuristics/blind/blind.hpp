#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_BLIND_HPP_

#include "../interface.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
struct BlindTag : public HeuristicTag { };


/**
 * Specialized implementation class.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
class Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>
    : public IHeuristic<Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;

    Problem m_problem;

    /* Implement IHeuristic interface. */
    template<typename>
    friend class IHeuristic;

    double compute_heuristic_impl(const StateView& state) {
        return 0.;
    }

public:
    Heuristic(Problem problem) : m_problem(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
