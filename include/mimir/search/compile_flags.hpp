#ifndef MIMIR_SEARCH_COMPILE_FLAGS_HPP_
#define MIMIR_SEARCH_COMPILE_FLAGS_HPP_

#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

#include <vector>

namespace mimir
{

// can use compile flags to define different representations
#ifdef FALSE
#else
using VState = mimir::ConstDenseStateViewProxy;
using VAction = mimir::ConstDenseActionViewProxy;
#endif

using VActionList = std::vector<VAction>;

}

#endif
