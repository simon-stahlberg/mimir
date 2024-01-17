#ifndef MIMIR_SEARCH_SEARCH_SPACE_HPP_
#define MIMIR_SEARCH_SEARCH_SPACE_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "search_node.hpp"

#include "../common/mixins.hpp"


namespace mimir {

template<typename ConfigType>
class SearchSpace {
private:
    // Container to store SearchNodes, i.e., PerStateInformation

public:
    /// @brief Gets the SearchNode of an existing state given by its id
    ///        or creates an default initialized SearchNode.
    SearchNode<ConfigType> get_or_create_node(ID<State<ConfigType>> state_id) {
        // TODO (Dominik): make this a reference to some memory.
        return SearchNode<ConfigType>();
    }
};

} // namespace mimir

#endif  // MIMIR_SEARCH_SEARCH_SPACE_HPP_

