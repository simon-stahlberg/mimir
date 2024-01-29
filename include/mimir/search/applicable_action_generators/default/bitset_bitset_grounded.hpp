#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_BITSET_BITSET_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_BITSET_BITSET_GROUNDED_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class AAG<AAGDispatcher<DefaultAAGTag, GroundedTag, BitsetStateTag, BitsetActionTag>>
   : public IAAG<AAG<AAGDispatcher<DefaultAAGTag, GroundedTag, BitsetStateTag, BitsetActionTag>>>
{
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;

    void generate_applicable_actions_impl(StateView state, std::vector<ActionView>& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;
};


}

#endif