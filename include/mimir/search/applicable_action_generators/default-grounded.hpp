#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_

#include "default.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class AAG<WrappedAAGTag<DefaultAAGTag, GroundedTag, BitsetStateTag>>
   : public AAGBase<AAG<WrappedAAGTag<DefaultAAGTag, GroundedTag, BitsetStateTag>>> {
    // Implement configuration specific functionality.
private:
    using StateView = View<WrappedStateTag<BitsetStateTag, LiftedTag>>;
    using ActionView = View<DefaultActionTag<LiftedTag, BitsetStateTag>>;

    void generate_applicable_actions_impl(StateView state, std::vector<ActionView>& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class AAGBase;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_
