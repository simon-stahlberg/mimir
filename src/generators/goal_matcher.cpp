#include "../../include/mimir/generators/goal_matcher.hpp"
#include "../../include/mimir/generators/lifted_schema_successor_generator.hpp"

#include <algorithm>
#include <vector>

namespace mimir::planners
{
    bool GoalMatcher::is_ground(const mimir::formalism::AtomList& goal)
    {
        for (const auto& atom : goal)
        {
            for (const auto& parameter : atom->arguments)
            {
                if (parameter->is_free_variable())
                {
                    return false;
                }
            }
        }

        return true;
    }

    GoalMatcher::GoalMatcher(const mimir::planners::StateSpace& state_space) :
        state_space_(state_space),
        state_distances_initial_(),
        state_distances_general_(),
        state_distance_mutex_()
    {
    }

    const std::vector<std::pair<mimir::formalism::State, int32_t>>& GoalMatcher::get_state_distances(const mimir::formalism::State& from_state)
    {
        std::lock_guard<std::mutex> lock(state_distance_mutex_);  // Lock is released automatically when it goes out of scope.
        std::equal_to<mimir::formalism::State> equal_to;

        const auto distance_ascending = [](const std::pair<mimir::formalism::State, int32_t>& lhs, const std::pair<mimir::formalism::State, int32_t>& rhs)
        { return lhs.second < rhs.second; };

        if (equal_to(from_state, state_space_->get_initial_state()))
        {
            if (state_distances_initial_.size() > 0)
            {
                return state_distances_initial_;
            }

            for (const auto& state : state_space_->get_states())
            {
                state_distances_initial_.emplace_back(state, state_space_->get_distance_from_initial_state(state));
            }

            std::sort(state_distances_initial_.begin(), state_distances_initial_.end(), distance_ascending);

            return state_distances_initial_;
        }
        else
        {
            state_distances_general_.clear();

            for (const auto& to_state : state_space_->get_states())
            {
                state_distances_general_.emplace_back(to_state, state_space_->get_distance_between_states(from_state, to_state));
            }

            std::sort(state_distances_general_.begin(), state_distances_general_.end(), distance_ascending);

            return state_distances_general_;
        }
    }

    std::pair<mimir::formalism::State, int32_t> GoalMatcher::best_match(const mimir::formalism::AtomList& goal)
    {
        return best_match(state_space_->get_initial_state(), goal);
    }

    std::pair<mimir::formalism::State, int32_t> GoalMatcher::best_match(const mimir::formalism::State& from_state, const mimir::formalism::AtomList& goal)
    {
        if (is_ground(goal))
        {
            const auto goal_ranks = state_space_->problem->to_ranks(goal);

            for (const auto& [to_state, distance] : get_state_distances(from_state))
            {
                if (mimir::formalism::subset_of_state(goal_ranks, to_state))
                {
                    return std::make_pair(to_state, distance);
                }
            }

            return std::make_pair(nullptr, -1);
        }
        else
        {
            std::equal_to<mimir::formalism::Object> equal_to;
            mimir::formalism::ParameterList parameters;
            mimir::formalism::LiteralList precondition;
            mimir::formalism::LiteralList effect;

            for (const auto& atom : goal)
            {
                for (const auto& term : atom->arguments)
                {
                    if (term->is_free_variable())
                    {
                        bool new_term = true;

                        for (const auto& parameter : parameters)
                        {
                            if (equal_to(parameter, term))
                            {
                                new_term = false;
                                break;
                            }
                        }

                        if (new_term)
                        {
                            parameters.emplace_back(term);
                        }
                    }
                }

                precondition.emplace_back(mimir::formalism::create_literal(atom, false));
            }

            const auto unit_cost = mimir::formalism::create_unit_cost_function(state_space_->domain);
            const auto action_schema = mimir::formalism::create_action_schema("dummy", parameters, precondition, effect, {}, unit_cost);
            mimir::planners::LiftedSchemaSuccessorGenerator successor_generator(action_schema, state_space_->problem);

            for (const auto& [to_state, distance] : get_state_distances(from_state))
            {
                const auto matches = successor_generator.get_applicable_actions(to_state);

                if (matches.size() > 0)
                {
                    return std::make_pair(to_state, distance);
                }
            }

            return std::make_pair(nullptr, -1);
        }
    }
}  // namespace planners
