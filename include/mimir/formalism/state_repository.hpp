#ifndef FORMALISM_STATE_REPOSITORY_HPP_
#define FORMALISM_STATE_REPOSITORY_HPP_

#include "declarations.hpp"
#include "problem.hpp"

#include <memory>

namespace mimir::formalism
{
    class StateRepositoryImpl
    {
      private:
        // Declare the copy constructor and copy assignment operator as deleted
        StateRepositoryImpl(const StateRepositoryImpl&) = delete;
        StateRepositoryImpl& operator=(const StateRepositoryImpl&) = delete;

        // Declare the move constructor and move assignment operator as deleted
        StateRepositoryImpl(StateRepositoryImpl&&) = delete;
        StateRepositoryImpl& operator=(StateRepositoryImpl&&) = delete;

        StateRepositoryImpl(const Problem& problem);

      public:
        virtual ~StateRepositoryImpl();

        State create_state(const AtomList& atoms);

        friend StateRepository create_state_repository(const Problem&);
    };

    StateRepository create_state_repository(const Problem& problem);

}  // namespace mimir::formalism

#endif  // FORMALISM_STATE_REPOSITORY_HPP_
