#ifndef MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_

#include "../applicable_action_generator.hpp"


namespace mimir
{

/// @brief Concrete implementation of a grounded successor generator.
template<>
class ApplicableActionGenerator<GroundedTag> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<GroundedTag>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<StateTag<GroundedTag>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<GroundedTag>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_
