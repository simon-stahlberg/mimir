#ifndef MIMIR_SEARCH_COMPILE_FLAGS_HPP_
#define MIMIR_SEARCH_COMPILE_FLAGS_HPP_

#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

// can use compile flags to define different representations
#ifdef FALSE
#else
using StateReprTag = DenseStateTag;
using State = ConstView<StateDispatcher<StateReprTag>>;
using GroundAction = ConstView<ActionDispatcher<StateReprTag>>;
#endif

}

#endif
