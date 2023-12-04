#ifndef MIMIR_PLANNERS_STATE_SPACE_HPP_
#define MIMIR_PLANNERS_STATE_SPACE_HPP_

#include "../formalism/declarations.hpp"
#include "../formalism/domain.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "../formalism/transition.hpp"
#include "successor_generator.hpp"

#include <limits>
#include <vector>

namespace mimir::planners
{
    class StateSpaceImpl;
    using StateSpace = std::shared_ptr<StateSpaceImpl>;
    using StateSpaceList = std::vector<StateSpace>;
    using StateSpaceSample = std::pair<mimir::formalism::State, StateSpace>;
    using StateSpaceSampleList = std::vector<StateSpaceSample>;

    struct StateInfo;

    class StateSpaceImpl
    {
      protected:
        StateSpaceImpl(const mimir::formalism::ProblemDescription& problem);

      public:
        mimir::formalism::DomainDescription domain;
        mimir::formalism::ProblemDescription problem;

        virtual ~StateSpaceImpl();

        virtual const std::vector<mimir::formalism::Transition>& get_forward_transitions(const mimir::formalism::State& state) const;

        virtual const std::vector<mimir::formalism::Transition>& get_backward_transitions(const mimir::formalism::State& state) const;

        virtual const std::vector<mimir::formalism::State>& get_states() const;

        virtual mimir::formalism::State get_initial_state() const;

        virtual uint64_t get_unique_index_of_state(const mimir::formalism::State& state) const;

        virtual bool is_dead_end_state(const mimir::formalism::State& state) const;

        virtual bool is_goal_state(const mimir::formalism::State& state) const;

        virtual int32_t get_distance_to_goal_state(const mimir::formalism::State& state) const;

        virtual int32_t get_distance_between_states(const mimir::formalism::State& from_state, const mimir::formalism::State& to_state) const;

        virtual int32_t get_longest_distance_to_goal_state() const;

        virtual int32_t get_distance_from_initial_state(const mimir::formalism::State& state) const;

        virtual mimir::formalism::State sample_state() const;

        virtual mimir::formalism::State sample_state_with_distance_to_goal(int32_t distance) const;

        virtual mimir::formalism::State sample_dead_end_state() const;

        virtual std::vector<mimir::formalism::State> get_goal_states() const;

        virtual uint64_t num_states() const;

        virtual uint64_t num_transitions() const;

        virtual uint64_t num_goal_states() const;

        virtual uint64_t num_dead_end_states() const;
    };

    std::ostream& operator<<(std::ostream& os, const mimir::planners::StateSpace& state_space);

    std::ostream& operator<<(std::ostream& os, const mimir::planners::StateSpaceList& state_spaces);

}  // namespace planners

#endif  // MIMIR_PLANNERS_STATE_SPACE_HPP_
