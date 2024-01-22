#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class.
*/
template<typename C>
requires IsConfig<C>
struct Zero { };


/**
 * Implementation class
*/
template<typename C>
requires IsConfig<C>
class Heuristic<Zero<C>> : public HeuristicBase<Heuristic<Zero<C>>> {
private:
    double compute_heuristic_impl(const View<State<C>>& state) {
        return 0.;
    }

    friend class HeuristicBase<Heuristic<Zero<C>>>;

public:
    Heuristic(Problem problem) : HeuristicBase<Heuristic<Zero<C>>>(problem) { }
};


/**
 * Type traits.
*/
template<typename C>
requires IsConfig<C>
struct TypeTraits<Heuristic<Zero<C>>> {
    using Config = C;
};


/**
 * Aliases
*/
template<typename C>
requires IsConfig<C>
using ZeroHeuristic = Heuristic<Zero<C>>;


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
