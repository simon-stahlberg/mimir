#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class
*/
template<Config C>
struct ZeroHeuristicTag { };


/**
 * Implementation class
*/
template<Config C>
class Heuristic<ZeroHeuristicTag<C>> : public HeuristicBase<Heuristic<ZeroHeuristicTag<C>>> {
private:
    double compute_heuristic_impl(const View<StateTag<C>>& state) {
        return 0.;
    }

    friend class HeuristicBase<Heuristic<ZeroHeuristicTag<C>>>;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<ZeroHeuristicTag<C>>>(problem) { }
};


template<Config C>
struct TypeTraits<Heuristic<ZeroHeuristicTag<C>>> {
    using ConfigTag = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
