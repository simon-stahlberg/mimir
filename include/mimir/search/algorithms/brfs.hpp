#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_base.hpp"
#include "../search_space.hpp"

#include <deque>


namespace mimir
{

/// @brief A general implementation of a breadth-first-search.
/// @tparam Config
template<typename Config>
class BrFS : public AlgorithmBase<BrFS<Config>> {
private:
    // Implement configuration independent functionality.
    std::deque<ID<State<Config>>> m_queue;

    void find_solution_impl() {
        // Use explicit types to make intellisense work, auto won't work:
        const State<Config>& initial_state = this->m_initial_state;
        SearchSpace<Config>& search_space = this->m_search_space;
        StateRepository<Config>& state_repository = this->m_state_repository;
        SuccessorGenerator<Config>& successor_generator = this->m_successor_generator;

        ID<State<Config>> initial_state_id = initial_state.get_id();
        SearchNode<Config> initial_search_node = search_space.get_or_create_node(initial_state_id);  // TODO (Dominik): make this a reference

        m_queue.push_back(initial_state_id);
        while (!m_queue.empty()) {
            auto state_id = m_queue.front();
            m_queue.pop_front();

            const State<Config> state = state_repository.lookup_state(state_id);
            const SearchNode<Config> search_node = search_space.get_or_create_node(state.get_id());

            const auto applicable_actions = successor_generator.generate_applicable_actions(state);
            for (const auto& action : applicable_actions) {
                const State<Config> successor_state = state_repository.get_or_create_successor_state(state, action);

                // TODO (Dominik): implement rest
            }
        }
    }

    friend class AlgorithmBase<BrFS<Config>>;

public:
    BrFS(Problem problem)
        : AlgorithmBase<BrFS<Config>>(problem) { }
};


template<typename Config>
struct TypeTraits<BrFS<Config>> {
    using ConfigType = Config;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
