#ifndef MIMIR_SEARCH_SEARCH_SPACE_HPP_
#define MIMIR_SEARCH_SEARCH_SPACE_HPP_

#include "declarations.hpp"
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


public:
    /// @brief Gets the SearchNode of an existing state given by its id
    ///        or creates an default initialized SearchNode.
    [[nodiscard]] SearchNode<C> get_or_create_node(State<C> state) {
        return nullptr;
    }
};

} // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_SPACE_HPP_

