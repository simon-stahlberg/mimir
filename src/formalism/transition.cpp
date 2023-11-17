/*
 * Copyright (C) 2023 Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../../include/mimir/formalism/transition.hpp"
#include "help_functions.hpp"

namespace mimir::formalism
{
    TransitionImpl::TransitionImpl(const mimir::formalism::State& source_state,
                                   const mimir::formalism::Action& action,
                                   const mimir::formalism::State& target_state) :
        source_state(source_state),
        target_state(target_state),
        action(action)
    {
    }

    Transition
    create_transition(const mimir::formalism::State& source_state, const mimir::formalism::Action& action, const mimir::formalism::State& target_state)
    {
        return std::make_shared<TransitionImpl>(source_state, action, target_state);
    }

    StateTransitions to_state_transitions(const mimir::formalism::ProblemDescription& problem, const mimir::formalism::TransitionList& transitions)
    {
        if (transitions.size() == 0)
        {
            throw std::invalid_argument("transitions is empty");
        }

        const auto& state = transitions[0]->source_state;
        StateList successors;

        for (const auto& transition : transitions)
        {
            if (transition->source_state != state)
            {
                throw std::invalid_argument("source states do not match");
            }

            successors.push_back(transition->target_state);
        }

        return std::make_tuple(state, std::move(successors), problem);
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Transition& transition)
    {
        os << transition->action;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::TransitionList& transitions)
    {
        print_vector(os, transitions);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Transition>::operator()(const mimir::formalism::Transition& transition) const
    {
        return hash_combine(transition->action, transition->source_state, transition->target_state);
    }

    bool less<mimir::formalism::Transition>::operator()(const mimir::formalism::Transition& left_transition,
                                                        const mimir::formalism::Transition& right_transition) const
    {
        return less_combine(std::make_tuple(left_transition->action, left_transition->source_state, left_transition->target_state),
                            std::make_tuple(right_transition->action, right_transition->source_state, right_transition->target_state));
    }

    bool equal_to<mimir::formalism::Transition>::operator()(const mimir::formalism::Transition& left_transition,
                                                            const mimir::formalism::Transition& right_transition) const
    {
        return equal_to_combine(std::make_tuple(left_transition->action, left_transition->source_state, left_transition->target_state),
                                std::make_tuple(right_transition->action, right_transition->source_state, right_transition->target_state));
    }
}  // namespace std
