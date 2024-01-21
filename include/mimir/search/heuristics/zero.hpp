#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic.hpp"


namespace mimir
{

/**
 * ID class
*/
template<Config C>
struct ZeroHeuristicID { };


/**
 * Implementation class
*/
template<Config C>
class Heuristic<ZeroHeuristicID<C>> : public HeuristicBase<Heuristic<ZeroHeuristicID<C>>> {
private:
    double compute_heuristic_impl(const View<State<C>>& state) {
        return 0.;
    }

    friend class HeuristicBase<Heuristic<ZeroHeuristicID<C>>>;
};


template<Config C>
struct TypeTraits<Heuristic<ZeroHeuristicID<C>>> {
    using ConfigType = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
