#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_

#include "../default.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class ApplicableActionGenerator<WrappedApplicableActionGeneratorTag<DefaultApplicableActionGeneratorTag, LiftedTag>>
   : public ApplicableActionGeneratorBase<ApplicableActionGenerator<WrappedApplicableActionGeneratorTag<DefaultApplicableActionGeneratorTag, LiftedTag>>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<State<LiftedTag>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<WrappedApplicableActionGeneratorTag<DefaultApplicableActionGeneratorTag, LiftedTag>>>;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_
