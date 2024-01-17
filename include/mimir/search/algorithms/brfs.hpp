#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"
#include "../state_base.hpp"

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
        auto initial_state_id = this->get_initial_state().get_id();
        auto initial_search_node = this->get_search_space().get_or_create_node(initial_state_id);  // TODO (Dominik): make this a reference

        m_queue.push_back(initial_state_id);
        while (!m_queue.empty()) {
            auto state_id = m_queue.front();
            m_queue.pop_front();

            // const auto applicable_actions = m_successor_generator.generate_applicable_actions()
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
