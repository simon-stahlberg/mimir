#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic_base.hpp"


namespace mimir
{

/// @brief A concrete heuristic.
template<typename Config>
class ZeroHeuristic : public HeuristicBase<ZeroHeuristic<Config>> {
private:
    double compute_heuristic_impl(const View<State<Config>>& state) {
        return 0.;
    }

    friend class HeuristicBase<ZeroHeuristic<Config>>;
};


template<typename Config>
struct TypeTraits<ZeroHeuristic<Config>> {
    using ConfigType = Config;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
