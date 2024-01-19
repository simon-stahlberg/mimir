#ifndef MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_

#include "../config.hpp"
#include "../declarations.hpp"
#include "../successor_generator_base.hpp"
#include "../../common/mixins.hpp"


namespace mimir
{

/// @brief Concrete implementation of a lifted successor generator.
template<>
class SuccessorGenerator<Lifted> : public SuccessorGeneratorBase<SuccessorGenerator<Lifted>> {
    // Implement configuration specific functionality.
private:
    GroundActionList generate_applicable_actions_impl(const View<State<Lifted>> state) {
        return GroundActionList();
    }

    // Give access to the private interface implementations.
    friend class SuccessorGeneratorBase<SuccessorGenerator<Lifted>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_SUCCESSOR_GENERATOR_HPP_
