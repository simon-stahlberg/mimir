#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_

#include "../default.hpp"


namespace mimir
{

/**
 * Fully specialized implementation class.
*/
template<>
class ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Lifted>> : public ApplicableActionGeneratorBase<ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Lifted>>> {
    // Implement configuration specific functionality.
private:
    void generate_applicable_actions_impl(View<State<Lifted>> state, GroundActionList& out_applicable_actions) {
    }

    // Give access to the private interface implementations.
    friend class ApplicableActionGeneratorBase<ApplicableActionGenerator<DefaultApplicableActionGeneratorTag<Lifted>>>;
};




}  // namespace mimir

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DEFAULT_LIFTED_HPP_
