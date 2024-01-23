#ifndef MIMIR_SEARCH_STATE_TAG_HPP_
#define MIMIR_SEARCH_STATE_TAG_HPP_

#include "config.hpp"

#include <cstdint>


namespace mimir
{

/**
 * ID class derive from it to provide specialized implementation for a state.
*/
struct StateBaseTag {};

/**
 *
*/
template<IsPlanningMode C>
struct State : public StateBaseTag { };

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
