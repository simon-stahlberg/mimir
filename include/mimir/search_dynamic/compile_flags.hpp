#ifndef MIMIR_SEARCH_DYNAMIC_COMPILE_FLAGS_HPP_
#define MIMIR_SEARCH_DYNAMIC_COMPILE_FLAGS_HPP_

#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

#include <vector>

namespace mimir::dynamic
{

// can use compile flags to define different representations
#ifdef FALSE
#else
using State = mimir::ConstBitsetStateViewProxy;
using Action = mimir::ConstBitsetActionViewProxy;
#endif

using ActionList = std::vector<Action>;

}

#endif
