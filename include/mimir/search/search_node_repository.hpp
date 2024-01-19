#ifndef MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_

#include "config.hpp"
#include "declarations.hpp"
#include "search_node_builder.hpp"
#include "state_base.hpp"
#include "state_builder_base.hpp"
#include "type_traits.hpp"

#include "../buffers/segmented_binary_vector.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<Config C>
class SearchNodeRepository : public UncopyableMixin<SearchNodeRepository<C>> {
private:
    // Persistent storage
    SegmentedBinaryVector<100000> m_data;

    // Reuse memory to create successor states.
    SearchNodeBuilder<C> m_state_builder;

public:
    [[nodiscard]] SearchNode<C> get_or_create_search_node(State<C> state) {
        // TODO (Dominik): implement
        return nullptr;
    }
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_
