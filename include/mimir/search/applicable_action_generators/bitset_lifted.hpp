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
    using ConstStateView = ConstView<StateDispatcher<BitsetStateTag, LiftedTag>>;
    using ConstActionView = ConstView<ActionDispatcher<LiftedTag, BitsetStateTag>>;

    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    void generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class IAAG;

public:
    AAG(Problem problem, PDDLFactories& pddl_factories) 
        : m_problem(problem)
        , m_pddl_factories(pddl_factories) {}
};

}  // namespace mimir

#endif
