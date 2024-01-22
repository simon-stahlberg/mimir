#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * Implementation class
*/
template<Config C>
class ZeroHeuristic : public HeuristicBase<ZeroHeuristic<C>> {
private:
    double compute_heuristic_impl(const View<StateTag<C>>& state) {
        return 0.;
    }

    friend class HeuristicBase<ZeroHeuristic<C>>;

public:
    ZeroHeuristic(Problem problem) : HeuristicBase<ZeroHeuristic<C>>(problem) { }
};


template<Config C>
struct TypeTraits<ZeroHeuristic<C>> {
    using ConfigType = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
