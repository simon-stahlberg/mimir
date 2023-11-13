#if !defined(PLANNERS_GOAL_GENERATOR_HPP_)
#define PLANNERS_GOAL_GENERATOR_HPP_

#include "../formalism/atom.hpp"
#include "../formalism/state.hpp"
#include "state_space.hpp"

namespace planners
{
    class GoalMatcher
    {
      private:
        planners::StateSpace state_space_;
        std::vector<std::pair<formalism::State, int32_t>> state_distances_;

        bool is_ground(const formalism::AtomList& goal);

      public:
        GoalMatcher(const planners::StateSpace& state_space);

        std::pair<formalism::State, int32_t> best_match(const formalism::AtomList& goal);
    };
}  // namespace planners

#endif  // PLANNERS_GOAL_GENERATOR_HPP_
