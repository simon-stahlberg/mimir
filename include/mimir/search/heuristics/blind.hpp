#ifndef MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
#define MIMIR_SEARCH_HEURISTICS_BLIND_HPP_

#include "mimir/search/heuristics/interface.hpp"

namespace mimir
{

/**
 * Specialized implementation class.
 */
class BlindHeuristic : public IHeuristic
{
private:
    Problem m_problem;

public:
    BlindHeuristic(Problem problem) : m_problem(problem) {}

    double compute_heuristic(VState state) override { return 0.; }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTICS_BLIND_HPP_
