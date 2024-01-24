#ifndef MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
#define MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_

#include "../config.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

#include "../../buffer/view_base.hpp"
#include "../../buffer/byte_stream_utils.hpp"


namespace mimir {

/**
 * ID class
*/
struct ActionBaseTag {};

template<typename DerivedTag>
concept IsActionTag = std::derived_from<DerivedTag, ActionBaseTag>;


/**
 * Wrapper class.
 *
 * Wrap the tag and the planning mode to be able use a given planning mode.
*/
template<IsActionTag A, IsPlanningModeTag P, IsStateTag S>
struct WrappedActionTag {
    using ActionTag = A;
    using PlanningModeTag = P;
    using StateTag = S;
};

template<typename T>
concept IsWrappedActionTag = requires {
    typename T::ActionTag;
    typename T::PlanningModeTag;
    typename T::StateTag;
};


} // namespace mimir



#endif  // MIMIR_SEARCH_ACTIONS_TEMPLATE_HPP_
