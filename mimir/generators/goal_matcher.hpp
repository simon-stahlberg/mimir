#if !defined(PLANNERS_GOAL_GENERATOR_HPP_)
#define PLANNERS_GOAL_GENERATOR_HPP_

#include "../formalism/atom.hpp"
#include "../formalism/state.hpp"
#include "state_space.hpp"

#include <mutex>

namespace planners
{
    class GoalMatcher
    {
      private:
        planners::StateSpace state_space_;
        std::vector<std::pair<formalism::State, int32_t>> state_distances_initial_;
        std::vector<std::pair<formalism::State, int32_t>> state_distances_general_;
        std::mutex state_distance_mutex_;

        const std::vector<std::pair<formalism::State, int32_t>>& get_state_distances(const formalism::State& state);

        bool is_ground(const formalism::AtomList& goal);

      public:
        GoalMatcher(const planners::StateSpace& state_space);

        std::pair<formalism::State, int32_t> best_match(const formalism::AtomList& goal);

        std::pair<formalism::State, int32_t> best_match(const formalism::State& from_state, const formalism::AtomList& goal);
    };
}  // namespace planners

#endif  // PLANNERS_GOAL_GENERATOR_HPP_
