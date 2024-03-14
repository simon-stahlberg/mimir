#ifndef MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_
#define MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/compile_flags.hpp"

namespace mimir
{

/**
 * Interface class
 */
class IHeuristic
{
public:
    virtual ~IHeuristic() = default;

    [[nodiscard]] virtual double compute_heuristic(VState state) = 0;
};

}

#endif