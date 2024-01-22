#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * Implementation class
*/
template<typename C>
requires IsConfig<C>
class ZeroHeuristic : public HeuristicBase<ZeroHeuristic<C>> {
private:
    double compute_heuristic_impl(const View<State<C>>& state) {
        return 0.;
    }

    friend class HeuristicBase<ZeroHeuristic<C>>;

public:
    ZeroHeuristic(Problem problem) : HeuristicBase<ZeroHeuristic<C>>(problem) { }
};


/**
 * Type traits.
*/
template<typename C>
requires IsConfig<C>
struct TypeTraits<ZeroHeuristic<C>> {
    using Config = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
