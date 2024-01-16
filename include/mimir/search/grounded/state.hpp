#ifndef MIMIR_SEARCH_GROUNDED_STATE_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_HPP_

#include "../config.hpp"
#include "../state_base.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief Concrete implementation of a grounded state.
template<>
class State<Grounded> : public StateBase<State<Grounded>> {
    // Implement configuration specific functionality.
private:

public:
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_HPP_
