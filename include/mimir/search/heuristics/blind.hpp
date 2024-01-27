#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "template.hpp"


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
struct BlindTag : public HeuristicBaseTag { };


/**
 * Dispatcher class.
 *
 * Define the required template arguments of your implementation.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct is_heuristic_dispatcher<HeuristicDispatcher<BlindTag, P, S, A>> : std::true_type {};


/**
 * Specialized implementation class.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
class Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>
    : public HeuristicBase<Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;

    double compute_heuristic_impl(const StateView& state) {
        return 0.;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class HeuristicBase;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>>(problem) { }
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
