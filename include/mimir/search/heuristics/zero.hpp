#ifndef MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
#define MIMIR_SEARCH_HEURISTICS_ZERO_HPP_

#include "../heuristic_base.hpp"


namespace mimir
{

/// @brief A concrete heuristic.
template<typename Configuration>
class ZeroHeuristic : public HeuristicBase<ZeroHeuristic<Configuration>> {
private:
    double compute_heuristic_impl(const State<Configuration>& state) {
        return 0.;
    }

    friend class HeuristicBase<ZeroHeuristic<Configuration>>;
};


template<typename Configuration>
struct TypeTraits<ZeroHeuristic<Configuration>> {
    using ConfigurationType = Configuration;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_ZERO_HPP_
