#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_view.hpp"

#include <deque>


namespace mimir
{

/**
 * ID class.
*/
template<Config C>
struct BrFSAlgorithmTag { };


/**
 * Implementation class.
*/
template<Config C>
class Algorithm<BrFSAlgorithmTag<C>> : public AlgorithmBase<Algorithm<BrFSAlgorithmTag<C>>> {
private:
    // Implement configuration independent functionality.
    std::deque<View<StateTag<C>>> m_queue;


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

    friend class AlgorithmBase<Algorithm<BrFSAlgorithmTag<C>>>;

public:
    Algorithm(const Problem& problem)
        : AlgorithmBase<Algorithm<BrFSAlgorithmTag<C>>>(problem) { }
};


/**
 * Type traits.
*/
template<Config C>
struct TypeTraits<Algorithm<BrFSAlgorithmTag<C>>> {
    using ConfigTag = C;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
