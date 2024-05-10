#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/type_traits.hpp"

namespace mimir
{

/**
 * Specialized implementation class.
 */
template<>
class Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>> : public IStaticHeuristic<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>
{
private:
    /* Implement IStaticHeuristic interface. */
    friend class IStaticHeuristic<Heuristic<HeuristicDispatcher<BlindTag, DenseStateTag>>>;

    double compute_heuristic_impl(const DenseState& state) { return 0.; }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
