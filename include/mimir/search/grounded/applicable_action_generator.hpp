#ifndef MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_
#define MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_

#include "../config.hpp"
#include "../declarations.hpp"
#include "../applicable_action_generator.hpp"


namespace mimir
{

/// @brief Concrete implementation of a grounded successor generator.
template<>
class ApplicableActionGenerator<Grounded> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<Grounded>> {
    // Implement configuration specific functionality.
private:
    GroundActionList generate_applicable_actions_impl(View<State<Grounded>> state) {
        return GroundActionList();
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<Grounded>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_APPLICABLE_ACTION_GENERATOR_HPP_
