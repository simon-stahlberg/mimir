#ifndef MIMIR_FORMALISM_TRANSITION_HPP_
#define MIMIR_FORMALISM_TRANSITION_HPP_

#include "action.hpp"
#include "state.hpp"

#include "../common/mixins.hpp"

#include <memory>


namespace mimir::formalism
{
    class Transition
    {
      public:
        State source_state;
        State target_state;
        Action action;

        Transition(const State& source_state, const Action& action, const State& target_state);
    };

    Transition create_transition(const State& source_state, const Action& action, const State& target_state);

    // StateTransitions to_state_transitions(const Problem& problem, const TransitionList& transitions);

    std::ostream& operator<<(std::ostream& os, const Transition& transition);

    using TransitionList = std::vector<Transition>;

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Transition>
    {
        std::size_t operator()(const mimir::formalism::Transition& transition) const;
    };

    template<>
    struct less<mimir::formalism::Transition>
    {
        bool operator()(const mimir::formalism::Transition& left_transition, const mimir::formalism::Transition& right_transition) const;
    };

    template<>
    struct equal_to<mimir::formalism::Transition>
    {
        bool operator()(const mimir::formalism::Transition& left_transition, const mimir::formalism::Transition& right_transition) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_TRANSITION_HPP_
