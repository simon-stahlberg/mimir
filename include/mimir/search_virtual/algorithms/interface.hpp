#ifndef MIMIR_SEARCH_VIRTUAL_ALGORITHMS_INTERFACE_HPP_
#define MIMIR_SEARCH_VIRTUAL_ALGORITHMS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search_virtual/compile_flags.hpp"

namespace mimir::runtime
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