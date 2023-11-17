#ifndef MIMIR_PLANNERS_GOAL_GENERATOR_HPP_
#define MIMIR_PLANNERS_GOAL_GENERATOR_HPP_

#include "../formalism/atom.hpp"
#include "../formalism/state.hpp"
#include "state_space.hpp"

#include <mutex>

namespace mimir::planners
{
    class GoalMatcher
    {
      private:
        mimir::planners::StateSpace state_space_;
        std::vector<std::pair<mimir::formalism::State, int32_t>> state_distances_initial_;
        std::vector<std::pair<mimir::formalism::State, int32_t>> state_distances_general_;
        std::mutex state_distance_mutex_;

        const std::vector<std::pair<mimir::formalism::State, int32_t>>& get_state_distances(const mimir::formalism::State& state);

        bool is_ground(const mimir::formalism::AtomList& goal);

      public:
        GoalMatcher(const mimir::planners::StateSpace& state_space);

        std::pair<mimir::formalism::State, int32_t> best_match(const mimir::formalism::AtomList& goal);

        std::pair<mimir::formalism::State, int32_t> best_match(const mimir::formalism::State& from_state, const mimir::formalism::AtomList& goal);
    };
}  // namespace planners

#endif  // MIMIR_PLANNERS_GOAL_GENERATOR_HPP_
