#ifndef MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_

#include "../config.hpp"
#include "../declarations.hpp"
#include "../applicable_action_generator.hpp"


namespace mimir
{

/// @brief Concrete implementation of a lifted successor generator.
template<>
class ApplicableActionGenerator<Lifted> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<Lifted>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<State<Lifted>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<Lifted>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_
