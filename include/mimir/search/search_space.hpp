#ifndef MIMIR_SEARCH_SEARCH_SPACE_HPP_
#define MIMIR_SEARCH_SEARCH_SPACE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "search_node.hpp"

#include "../common/mixins.hpp"


namespace mimir {

template<typename Config>
class SearchSpace {
private:
    // Container to store SearchNodes, i.e., PerStateInformation

    // TODO (Dominik): just temporary to get a reference returned. implement the storage
    SearchNode<Config> some_search_node;

public:
    /// @brief Gets the SearchNode of an existing state given by its id
    ///        or creates an default initialized SearchNode.
    SearchNode<Config>& get_or_create_node(ID<State<Config>> state_id) {
        // TODO (Dominik): make this a reference to some memory.
        return some_search_node;
    }
};

} // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_SPACE_HPP_

