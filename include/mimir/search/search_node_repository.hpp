#ifndef MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_

#include "config.hpp"
#include "declarations.hpp"
#include "search_node_builder.hpp"
#include "state_view_base.hpp"
#include "state_builder_base.hpp"
#include "type_traits.hpp"

#include "../buffer/char_stream_segmented.hpp"
#include "../buffer/containers/vector.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <cassert>
#include <unordered_map>


namespace mimir
{



// TODO (Dominik): Make this InformationByState or PerStateInformation and templatize it by the type

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<Config C>
class SearchNodeRepository : public UncopyableMixin<SearchNodeRepository<C>> {
private:
    // Persistent storage
    // AutomaticVector m_data;

public:
    [[nodiscard]] View<SearchNode<C>> get_or_create_search_node(const View<State<C>> state) {
        state.get_id();
        int state_id = state->get_id();
        // If with index i is new then we already created search nodes for states with index 0,...,i-1
        assert(state_id <= static_cast<int>(m_node_by_state_id.size()));
        if (state_id < static_cast<int>(m_node_by_state_id.size())) {
            return m_node_by_state_id[state_id];
        }
        m_search_node_builder.clear();
        m_search_node_builder.finish();
        auto search_node = reinterpret_cast<SearchNode<C>>(
            m_data.write(
                m_search_node_builder.get_buffer_pointer(),
                m_search_node_builder.get_size()));
        m_node_by_state_id.push_back(search_node);
        return search_node;
    }
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_NODE_REPOSITORY_HPP_
