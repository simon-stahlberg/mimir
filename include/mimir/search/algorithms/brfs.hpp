#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "template.hpp"

#include <deque>
#include <vector>


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P
       , IsStateTag S = BitsetStateTag
       , IsActionTag A = DefaultActionTag
       , IsAAGTag AG = DefaultAAGTag
       , IsSSGTag SG = DefaultSSGTag>
struct BrFSTag : public AlgorithmBaseTag { };


/**
 * Dispatcher class.
 *
 * Define the required template arguments of your implementation.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A, IsAAGTag AG, IsSSGTag SG>
struct is_algorithm_dispatcher<AlgorithmDispatcher<BrFSTag<P, S, A, AG, SG>>> : std::true_type {};


/**
 * Specialized implementation class.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A, IsAAGTag AG, IsSSGTag SG>
class Algorithm<AlgorithmDispatcher<BrFSTag<P, S, A, AG, SG>>>
    : public AlgorithmBase<Algorithm<AlgorithmDispatcher<BrFSTag<P, S, A, AG, SG>>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;
    using ActionViewList = std::vector<ActionView>;

    // Implement configuration independent functionality.
    std::deque<StateView> m_queue;

    AutomaticVector<CostSearchNodeTag<P, S, A>> m_search_nodes;


    SearchStatus find_solution_impl(ActionViewList& out_plan) {
        auto initial_search_node = this->m_search_nodes[this->m_initial_state.get_id()];
        // TODO (Dominik): update the data of the initial_search_node

        auto applicable_actions = ActionViewList();

        m_queue.push_back(this->m_initial_state);
        while (!m_queue.empty()) {
            const auto state = m_queue.front();
            m_queue.pop_front();

            const auto search_node = this->m_search_nodes[state.get_id()];

            this->m_successor_generator.generate_applicable_actions(state, applicable_actions);
            for (const auto& action : applicable_actions) {
                const auto& successor_state = this->m_state_repository.get_or_create_successor_state(state, action);

                // TODO (Dominik): implement rest
            }
        }
        return SearchStatus::FAILED;
    }

    // Give access to the private interface implementations.
    template<typename>
    friend class AlgorithmBase;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<AlgorithmDispatcher<BrFSTag<P, S, A, AG, SG>>>>(problem)
        , m_search_nodes(AutomaticVector(
            Builder<CostSearchNodeTag<P, S, A>>(
                SearchNodeStatus::CLOSED,
                0, StateView(nullptr),
                ActionView(nullptr)
                )))
        { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A, IsAAGTag AG, IsSSGTag SG>
struct TypeTraits<Algorithm<AlgorithmDispatcher<BrFSTag<P, S, A, AG, SG>>>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
    using AAGTag = AG;
    using SSGTag = SG;

    using ActionView = View<ActionDispatcher<A, P, S>>;
    using ActionViewList = std::vector<ActionView>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
