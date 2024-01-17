#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_base.hpp"
#include "../search_space.hpp"
#include "../state_repository_base.hpp"

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

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        auto initial_state_id = this->m_initial_state.get_id();
        auto& initial_search_node = this->m_search_space.get_or_create_node(initial_state_id);
        // TODO (Dominik): update the data of the initial_search_node

        auto applicable_actions = GroundActionList();

        m_queue.push_back(initial_state_id);
        while (!m_queue.empty()) {
            auto state_id = m_queue.front();
            m_queue.pop_front();

            const auto& state = this->m_state_repository.lookup_state(state_id);
            const auto search_node = this->m_search_space.get_or_create_node(state.get_id());

            this->m_successor_generator.generate_applicable_actions(state, applicable_actions);
            for (const auto& action : applicable_actions) {
                const auto& successor_state = this->m_state_repository.get_or_create_successor_state(state, action);

                // TODO (Dominik): implement rest
            }
        }
        return SearchStatus::FAILED;
    }

    friend class AlgorithmBase<BrFS<Config>>;

public:
    BrFS(const Problem& problem)
        : AlgorithmBase<BrFS<Config>>(problem) { }
};


template<typename Config>
struct TypeTraits<BrFS<Config>> {
    using ConfigType = Config;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
