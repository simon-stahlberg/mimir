#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_

#include "../default.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Grounded>> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Grounded>>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<State<Grounded>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Grounded>>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_GROUNDED_HPP_
