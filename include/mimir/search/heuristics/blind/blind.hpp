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
struct BlindTag : public HeuristicTag
{
};

/**
 * Specialized implementation class.
 */
template<IsPlanningModeTag P, IsStateTag S>
class Heuristic<HeuristicDispatcher<BlindTag, P, S>> : public IHeuristic<Heuristic<HeuristicDispatcher<BlindTag, P, S>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<S, P>>;

    Problem m_problem;

    /* Implement IHeuristic interface. */
    template<typename>
    friend class IHeuristic;

    double compute_heuristic_impl(const ConstStateView& state) { return 0.; }

public:
    Heuristic(Problem problem) : m_problem(problem) {}
};

/**
 * Type traits.
 */
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Heuristic<HeuristicDispatcher<BlindTag, P, S>>>
{
    using PlanningModeTag = P;
    using StateTag = S;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
