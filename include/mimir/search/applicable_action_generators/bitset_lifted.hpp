#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_BITSET_LIFTED_HPP_

#include "interface.hpp"

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<AAGDispatcher<LiftedTag, BitsetStateTag>> : public IAAG<AAG<AAGDispatcher<LiftedTag, BitsetStateTag>>>
{
private:
    using StateView = View<StateDispatcher<BitsetStateTag, LiftedTag>>;
    using ActionView = View<ActionDispatcher<LiftedTag, BitsetStateTag>>;

    void generate_applicable_actions_impl(StateView state, std::vector<ActionView>& out_applicable_actions)
    {
        out_applicable_actions.clear();
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;
};

}  // namespace mimir

#endif
