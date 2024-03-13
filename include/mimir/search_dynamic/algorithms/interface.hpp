#ifndef MIMIR_SEARCH_DYNAMIC_ALGORITHMS_INTERFACE_HPP_
#define MIMIR_SEARCH_DYNAMIC_ALGORITHMS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search_virtual/compile_flags.hpp"

namespace mimir::dynamic
{

/**
 * Interface class.
 */
class IAlgorithm
{
private:
public:
    virtual ~IAlgorithm() = default;

    virtual SearchStatus find_solution(ActionList& out_plan) = 0;
};

}
#endif