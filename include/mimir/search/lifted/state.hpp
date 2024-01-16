#ifndef MIMIR_SEARCH_LIFTED_STATE_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_HPP_

#include "../config.hpp"
#include "../state_base.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief Concrete implementation of a lifted state.
template<>
class State<Lifted> : public StateBase<State<Lifted>> {
    // Implement configuration specific functionality.
private:

public:
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_HPP_
