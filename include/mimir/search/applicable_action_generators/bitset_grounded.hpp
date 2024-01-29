#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_GROUNDED_HPP_

#include "interface.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class AAG<AAGDispatcher<GroundedTag, BitsetStateTag>>
   : public IAAG<AAG<AAGDispatcher<GroundedTag, BitsetStateTag>>>
{
private:
    using StateView = View<StateDispatcher<BitsetStateTag, GroundedTag>>;
    using ActionView = View<ActionDispatcher<GroundedTag, BitsetStateTag>>;

    void generate_applicable_actions_impl(StateView state, std::vector<ActionView>& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;
};


}

#endif