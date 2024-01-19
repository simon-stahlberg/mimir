#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_base.hpp"
#include "../search_node_repository.hpp"
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
    std::deque<State<Config>> m_queue;

    SearchStatus find_solution_impl(GroundActionList& out_plan) {
        auto initial_search_node = this->m_search_node_repository.get_or_create_search_node(this->m_initial_state);
        // TODO (Dominik): update the data of the initial_search_node

        m_queue.push_back(this->m_initial_state);
        while (!m_queue.empty()) {
            const auto state = m_queue.front();
            m_queue.pop_front();

            const auto search_node = this->m_search_node_repository.get_or_create_search_node(state);

            auto applicable_actions = this->m_successor_generator.generate_applicable_actions(state);
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
