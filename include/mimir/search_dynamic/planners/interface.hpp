#ifndef MIMIR_SEARCH_DYNAMIC_PLANNERS_INTERFACE_HPP_
#define MIMIR_SEARCH_DYNAMIC_PLANNERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/plan.hpp"

#include <loki/utils/filesystem.hpp>
#include <string>

namespace mimir::dynamic
{

/**
 * Interface class
 */
class IPlanner
{
public:
    virtual ~IPlanner() = default;

    virtual const Domain& get_domain() const = 0;
    virtual const Problem& get_problem() const = 0;

    virtual std::tuple<SearchStatus, Plan> find_solution() = 0;
};

}

#endif
