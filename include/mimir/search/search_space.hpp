#ifndef MIMIR_SEARCH_SEARCH_SPACE_HPP_
#define MIMIR_SEARCH_SEARCH_SPACE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "search_node.hpp"

#include "../common/mixins.hpp"


namespace mimir {

template<Config C>
class SearchSpace {
private:
    // Container to store SearchNodes, i.e., PerStateInformation

    // TODO (Dominik): just temporary to get a reference returned. implement the storage
    SearchNode<C> some_search_node;

public:
    /// @brief Gets the SearchNode of an existing state given by its id
    ///        or creates an default initialized SearchNode.
    [[nodiscard]] SearchNode<C>& get_or_create_node(ID<State<C>> state_id) {
        return some_search_node;
    }
};

} // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_SPACE_HPP_

