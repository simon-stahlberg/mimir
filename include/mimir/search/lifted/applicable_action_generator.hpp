#ifndef MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_

#include "../applicable_action_generator.hpp"


namespace mimir
{

/// @brief Concrete implementation of a lifted successor generator.
template<>
class ApplicableActionGenerator<LiftedTag> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<LiftedTag>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<StateTag<LiftedTag>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<LiftedTag>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_APPLICABLE_ACTION_GENERATOR_HPP_
