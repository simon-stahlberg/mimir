#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_view.hpp"

#include <deque>


namespace mimir
{

/**
 * Spezialized implementation class.
*/
template<typename C>
class BrFSAlgorithm : public AlgorithmBase<BrFSAlgorithm<C>> {
private:
    // Implement configuration independent functionality.
    std::deque<View<State<C>>> m_queue;


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

    friend class AlgorithmBase<BrFSAlgorithm<C>>;

public:
    BrFSAlgorithm(const Problem& problem)
        : AlgorithmBase<BrFSAlgorithm<C>>(problem) { }
};


/**
 * Type traits.
*/
template<typename C>
requires IsConfig<C>
struct TypeTraits<BrFSAlgorithm<C>> {
    using Config = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
