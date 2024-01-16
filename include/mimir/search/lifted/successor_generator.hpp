#ifndef MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_

#include "../successor_generator_base.hpp"

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

#endif  // MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_
