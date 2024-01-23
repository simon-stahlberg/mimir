#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_template.hpp"
#include "../state_view.hpp"

#include <deque>


namespace mimir
{

/**
 * ID class to dispatch a specialized implementation
*/
template<IsPlanningModeTag P>
struct BrFS : public AlgorithmBaseTag { };


/**
 * Spezialized implementation class.
*/
template<IsPlanningModeTag P>
class Algorithm<BrFS<P>> : public AlgorithmBase<Algorithm<BrFS<P>>> {
private:
    // Implement configuration independent functionality.
    std::deque<View<State<P>>> m_queue;


    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        auto initial_search_node = this->m_search_nodes[this->m_initial_state.get_id()];
        // TODO (Dominik): update the data of the initial_search_node

        auto applicable_actions = GroundActionList();

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

    friend class AlgorithmBase<Algorithm<BrFS<P>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<BrFS<P>>>(problem) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<BrFS<P>> {
    using PlanningMode = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<Algorithm<BrFS<P>>> {
    using PlanningMode = P;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
