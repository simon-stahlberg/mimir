#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_

#include "successor_generator.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief Concrete implementation of a lifted successor generator.
template<>
class SuccessorGenerator<Lifted> : public SuccessorGeneratorBase<SuccessorGenerator<Lifted>> {
private:
    ActionList generate_applicable_actions_impl(const State<Lifted>& state) {
        return ActionList();
    }

    // Give access to the private interface implementations.
    friend class SuccessorGeneratorBase<SuccessorGenerator<Lifted>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_
