#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_BRFS_HPP_

#include "../interface.hpp"

#include <flatmemory/flatmemory.hpp>

#include <deque>
#include <vector>


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P
       , IsStateTag S = BitsetStateTag>
struct BrFSTag : public AlgorithmTag { };


/**
 * Specialized implementation class.
*/

template<IsPlanningModeTag P, IsStateTag S>
class Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>>
    : public IAlgorithm<Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>>> {
private:
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<P, S>>;
    using ActionViewList = std::vector<ActionView>;

    Problem m_problem;
    SSG<SSGDispatcher<P, S>> m_state_repository;
    StateView m_initial_state;
    AAG<AAGDispatcher<P, S>> m_successor_generator;

    // Implement configuration independent functionality.
    std::deque<StateView> m_queue;

    flatmemory::FixedSizedTypeVector<flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t>> m_search_nodes;


    /* Implement IAlgorithm interface. */
    template<typename>
    friend class IAlgorithm;

    SearchStatus find_solution_impl(ActionViewList& out_plan) {
        auto initial_search_node = CostSearchNodeViewWrapper(this->m_search_nodes[this->m_initial_state.get_id()]);
        // TODO (Dominik): update the data of the initial_search_node
        initial_search_node.get_g_value() = 0;

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

    static auto create_default_search_node_builder() {
        flatmemory::Builder<flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t>> builder;
        builder.get_builder<0>() = SearchNodeStatus::CLOSED;
        builder.get_builder<1>() = -1;
        builder.get_builder<2>() = -1;
        builder.finish();
        return builder;  
    }

public:
    Algorithm(const Problem& problem)
        : m_problem(problem)
        , m_state_repository(SSG<SSGDispatcher<P, S>>())
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem))
        , m_successor_generator(AAG<AAGDispatcher<P, S>>())
        , m_search_nodes(flatmemory::FixedSizedTypeVector(create_default_search_node_builder())) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S>
struct TypeTraits<Algorithm<AlgorithmDispatcher<BrFSTag<P, S>>>> {
    using PlanningModeTag = P;
    using StateTag = S;

    using ActionView = View<ActionDispatcher<P, S>>;
    using ActionViewList = std::vector<ActionView>;
};


} 

#endif 
