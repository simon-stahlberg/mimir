#ifndef MIMIR_SEARCH_PLANNERS_INTERFACE_HPP_
#define MIMIR_SEARCH_PLANNERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/plan.hpp"

#include <loki/loki.hpp>
#include <string>

namespace mimir
{

/**
 * Interface class
 */
class IPlanner
{
public:
    virtual ~IPlanner() = default;

    virtual std::tuple<SearchStatus, Plan> find_solution() = 0;
};

}

#endif
