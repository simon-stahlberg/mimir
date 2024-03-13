#ifndef MIMIR_SEARCH_VIRTUAL_COMPILE_FLAGS_HPP_
#define MIMIR_SEARCH_VIRTUAL_COMPILE_FLAGS_HPP_

#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

#include <vector>

namespace mimir::runtime
{

// can use compile flags to define different representations
#ifdef FALSE
#else
using State = mimir::ConstBitsetStateView;
using Action = mimir::ConstBitsetActionView;
#endif

using ActionList = std::vector<Action>;

}

#endif
