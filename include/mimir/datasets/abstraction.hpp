/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#ifndef MIMIR_DATASETS_ABSTRACTION_HPP_
#define MIMIR_DATASETS_ABSTRACTION_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/datasets/abstract_transition.hpp"
#include "mimir/graphs/graph_iterators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"

#include <concepts>
#include <memory>

namespace mimir
{

/**
 * Internal concept
 */

template<typename T>
concept IsAbstraction = requires(T a, State concrete_state)
{
    {
        a.get_abstract_state_index(concrete_state)
        } -> std::same_as<StateIndex>;
};

/**
 * External inheritance hierarchy using type erasure.
 */

class Abstraction
{
private:
    // The External Polymorphism Design Pattern
    class AbstractionConcept
    {
    public:
        virtual ~AbstractionConcept() {}

        /* Abstraction */
        virtual StateIndex get_abstract_state_index(State concrete_state) const = 0;

        /* Meta data */
        virtual Problem get_problem() const = 0;

        /* Memory */
        virtual const std::shared_ptr<PDDLFactories>& get_pddl_factories() const = 0;
        virtual const std::shared_ptr<IAAG>& get_aag() const = 0;
        virtual const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const = 0;

        /* States */
        virtual StateIndex get_initial_state() const = 0;
        virtual const StateIndexSet& get_goal_states() const = 0;
        virtual const StateIndexSet& get_deadend_states() const = 0;
        virtual size_t get_num_states() const = 0;
        virtual size_t get_num_goal_states() const = 0;
        virtual size_t get_num_deadend_states() const = 0;
        virtual AdjacentVertexIndexIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_state_indices(StateIndex state) const = 0;
        virtual AdjacentVertexIndexIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_state_indices(StateIndex state) const = 0;
        virtual bool is_goal_state(StateIndex state) const = 0;
        virtual bool is_deadend_state(StateIndex state) const = 0;
        virtual bool is_alive_state(StateIndex state) const = 0;

        /* Transitions */
        virtual const AbstractTransitionList& get_transitions() const = 0;
        virtual TransitionCost get_transition_cost(TransitionIndex transition) const = 0;
        virtual AdjacentEdgeIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_transitions(StateIndex state) const = 0;
        virtual AdjacentEdgeIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_transitions(StateIndex state) const = 0;
        virtual AdjacentEdgeIndexIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_transition_indices(StateIndex state) const = 0;
        virtual AdjacentEdgeIndexIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_transition_indices(StateIndex state) const = 0;
        virtual size_t get_num_transitions() const = 0;

        /* Distances */
        virtual const std::vector<Distance>& get_goal_distances() const = 0;

        // The Prototype Design Pattern
        virtual std::unique_ptr<AbstractionConcept> clone() const = 0;
    };

    template<IsAbstraction A>
    class AbstractionModel : public AbstractionConcept
    {
    private:
        A m_abstraction;

    public:
        explicit AbstractionModel(A abstraction) : m_abstraction(std::move(abstraction)) {}

        /* Abstraction */
        StateIndex get_abstract_state_index(State concrete_state) const override { return m_abstraction.get_abstract_state_index(concrete_state); }

        /* Meta data */
        Problem get_problem() const override { return m_abstraction.get_problem(); }

        /* Memory */
        const std::shared_ptr<PDDLFactories>& get_pddl_factories() const override { return m_abstraction.get_pddl_factories(); }
        const std::shared_ptr<IAAG>& get_aag() const override { return m_abstraction.get_aag(); }
        const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const override { return m_abstraction.get_ssg(); }

        /* States */
        StateIndex get_initial_state() const override { return m_abstraction.get_initial_state(); }
        const StateIndexSet& get_goal_states() const override { return m_abstraction.get_goal_states(); }
        const StateIndexSet& get_deadend_states() const override { return m_abstraction.get_deadend_states(); }
        AdjacentVertexIndexIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_state_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_state_indices<ForwardTraversal>(state);
        }
        AdjacentVertexIndexIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_state_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_state_indices<BackwardTraversal>(state);
        }
        size_t get_num_states() const override { return m_abstraction.get_num_states(); }
        size_t get_num_goal_states() const override { return m_abstraction.get_num_goal_states(); }
        size_t get_num_deadend_states() const override { return m_abstraction.get_num_deadend_states(); }
        bool is_goal_state(StateIndex state) const override { return m_abstraction.is_goal_state(state); }
        bool is_deadend_state(StateIndex state) const override { return m_abstraction.is_deadend_state(state); }
        bool is_alive_state(StateIndex state) const override { return m_abstraction.is_alive_state(state); }

        /* Transitions */
        const AbstractTransitionList& get_transitions() const override { return m_abstraction.get_transitions(); }
        TransitionCost get_transition_cost(TransitionIndex transition) const override { return m_abstraction.get_transition_cost(transition); }
        AdjacentEdgeIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_transitions(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transitions<ForwardTraversal>(state);
        }
        AdjacentEdgeIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_transitions(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transitions<BackwardTraversal>(state);
        }
        AdjacentEdgeIndexIterator<AbstractTransition, ForwardTraversal> get_forward_adjacent_transition_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transition_indices<ForwardTraversal>(state);
        }
        AdjacentEdgeIndexIterator<AbstractTransition, BackwardTraversal> get_backward_adjacent_transition_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transition_indices<BackwardTraversal>(state);
        }
        size_t get_num_transitions() const override { return m_abstraction.get_num_transitions(); }

        /* Distances */
        const std::vector<Distance>& get_goal_distances() const override { return m_abstraction.get_goal_distances(); }

        // The Prototype Design Pattern
        std::unique_ptr<AbstractionConcept> clone() const override { return std::make_unique<AbstractionModel<A>>(*this); }
    };

    // The Bridge Design Pattern
    std::unique_ptr<AbstractionConcept> m_pimpl;

public:
    using TransitionType = AbstractTransition;

    template<IsAbstraction A>
    explicit Abstraction(A abstraction) : m_pimpl(std::make_unique<AbstractionModel<A>>(std::move(abstraction)))
    {
    }

    // Copy operations
    Abstraction(const Abstraction& other) : m_pimpl { other.m_pimpl->clone() } {}
    Abstraction& operator=(const Abstraction& other)
    {
        // Copy-and-swap idiom
        Abstraction tmp(other);
        std::swap(m_pimpl, tmp.m_pimpl);
        return *this;
    }

    // Move operations
    Abstraction(Abstraction&& other) noexcept = default;
    Abstraction& operator=(Abstraction&& other) noexcept = default;

    /* Abstraction */
    StateIndex get_abstract_state_index(State concrete_state) const { return m_pimpl->get_abstract_state_index(concrete_state); }

    /* Meta data */
    Problem get_problem() const { return m_pimpl->get_problem(); }

    /* Memory */
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const { return m_pimpl->get_pddl_factories(); }
    const std::shared_ptr<IAAG>& get_aag() const { return m_pimpl->get_aag(); }
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const { return m_pimpl->get_ssg(); }

    /* States */
    StateIndex get_initial_state() const { return m_pimpl->get_initial_state(); }
    const StateIndexSet& get_goal_states() const { return m_pimpl->get_goal_states(); }
    const StateIndexSet& get_deadend_states() const { return m_pimpl->get_deadend_states(); }
    template<IsTraversalDirection Direction>
    AdjacentVertexIndexIterator<AbstractTransition, Direction> get_adjacent_state_indices(StateIndex state) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_state_indices(state);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_state_indices(state);
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "Abstraction::get_adjacent_state_indices(...): Missing implementation for IsTraversalDirection.");
        }
    }
    size_t get_num_states() const { return m_pimpl->get_num_states(); }
    size_t get_num_goal_states() const { return m_pimpl->get_num_goal_states(); }
    size_t get_num_deadend_states() const { return m_pimpl->get_num_deadend_states(); }
    bool is_goal_state(StateIndex state) const { return m_pimpl->is_goal_state(state); }
    bool is_deadend_state(StateIndex state) const { return m_pimpl->is_deadend_state(state); }
    bool is_alive_state(StateIndex state) const { return m_pimpl->is_alive_state(state); }

    /* Transitions */
    // Write an adaptor if you need to return different kinds of transitions
    const AbstractTransitionList& get_transitions() const { return m_pimpl->get_transitions(); }
    TransitionCost get_transition_cost(TransitionIndex transition) const { return m_pimpl->get_transition_cost(transition); }
    template<IsTraversalDirection Direction>
    AdjacentEdgeIterator<AbstractTransition, Direction> get_adjacent_transitions(StateIndex state) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_transitions(state);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_transitions(state);
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "Abstraction::get_adjacent_transitions(...): Missing implementation for IsTraversalDirection.");
        }
    }
    template<IsTraversalDirection Direction>
    AdjacentEdgeIndexIterator<AbstractTransition, Direction> get_adjacent_transition_indices(StateIndex state) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_transition_indices(state);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_transition_indices(state);
        }
        else
        {
            static_assert(dependent_false<Direction>::value,
                          "Abstraction::get_adjacent_transition_indices(...): Missing implementation for IsTraversalDirection.");
        }
    }
    size_t get_num_transitions() const { return m_pimpl->get_num_transitions(); }

    /* Distances */
    const std::vector<Distance>& get_goal_distances() const { return m_pimpl->get_goal_distances(); }
};

static_assert(IsAbstraction<Abstraction>);

using AbstractionList = std::vector<Abstraction>;

}

#endif