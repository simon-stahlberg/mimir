#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/compile_flags.hpp"

namespace mimir
{
/**
 * Interface class
 */
class ISSG
{
public:
    virtual ~ISSG() = default;

    [[nodiscard]] virtual VState get_or_create_initial_state(Problem problem) = 0;

    [[nodiscard]] virtual VState get_or_create_successor_state(VState state, VAction action) = 0;

    [[nodiscard]] virtual size_t state_count() const = 0;
};
}

#endif
