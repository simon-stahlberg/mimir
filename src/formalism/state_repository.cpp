#include "../../include/mimir/formalism/state.hpp"
#include "../../include/mimir/formalism/state_repository.hpp"

namespace mimir::formalism
{
    StateRepositoryImpl::StateRepositoryImpl(const Problem& problem) {}

    StateRepositoryImpl::~StateRepositoryImpl() {}

    State StateRepositoryImpl::create_state(const AtomList& atoms) { throw std::runtime_error("not implemented"); }

    StateRepository create_state_repository(const Problem& problem) { return std::shared_ptr<StateRepositoryImpl>(new StateRepositoryImpl(problem)); }

}  // namespace mimir::formalism
