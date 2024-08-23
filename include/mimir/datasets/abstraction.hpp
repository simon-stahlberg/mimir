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
#include "mimir/datasets/ground_actions_edge.hpp"
#include "mimir/graphs/static_graph_interface.hpp"
#include "mimir/graphs/static_graph_iterators.hpp"
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

/// @brief `Abstraction` erases the type of a concrete implementation of an abstraction with type A.
/// The concrete abstraction type A is required to have an underlying static graph to ensure that the static iterators can be used.
/// The requirement is usually not an issue since abstraction do not require vertex or edge removal after construction.
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
        virtual const std::shared_ptr<IApplicableActionGenerator>& get_aag() const = 0;
        virtual const std::shared_ptr<StateRepository>& get_ssg() const = 0;

        /* States */
        virtual StateIndex get_initial_state() const = 0;
        virtual const StateIndexSet& get_goal_states() const = 0;
        virtual const StateIndexSet& get_deadend_states() const = 0;
        virtual size_t get_num_states() const = 0;
        virtual size_t get_num_goal_states() const = 0;
        virtual size_t get_num_deadend_states() const = 0;
        virtual std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_state_indices(StateIndex state) const = 0;
        virtual std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_state_indices(StateIndex state) const = 0;
        virtual bool is_goal_state(StateIndex state) const = 0;
        virtual bool is_deadend_state(StateIndex state) const = 0;
        virtual bool is_alive_state(StateIndex state) const = 0;

        /* Transitions */
        virtual const GroundActionsEdgeList& get_transitions() const = 0;
        virtual TransitionCost get_transition_cost(TransitionIndex transition) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_transitions(StateIndex state) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_transitions(StateIndex state) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_transition_indices(StateIndex state) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_transition_indices(StateIndex state) const = 0;
        virtual size_t get_num_transitions() const = 0;

        /* Distances */
        virtual const DistanceList& get_goal_distances() const = 0;
        virtual Distance get_goal_distance(StateIndex state) const = 0;

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
        const std::shared_ptr<IApplicableActionGenerator>& get_aag() const override { return m_abstraction.get_aag(); }
        const std::shared_ptr<StateRepository>& get_ssg() const override { return m_abstraction.get_ssg(); }

        /* States */
        StateIndex get_initial_state() const override { return m_abstraction.get_initial_state(); }
        const StateIndexSet& get_goal_states() const override { return m_abstraction.get_goal_states(); }
        const StateIndexSet& get_deadend_states() const override { return m_abstraction.get_deadend_states(); }
        std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_state_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_state_indices<ForwardTraversal>(state);
        }
        std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_state_indices(StateIndex state) const override
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
        const GroundActionsEdgeList& get_transitions() const override { return m_abstraction.get_transitions(); }
        TransitionCost get_transition_cost(TransitionIndex transition) const override { return m_abstraction.get_transition_cost(transition); }
        std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_transitions(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transitions<ForwardTraversal>(state);
        }
        std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_transitions(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transitions<BackwardTraversal>(state);
        }
        std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_transition_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transition_indices<ForwardTraversal>(state);
        }
        std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_transition_indices(StateIndex state) const override
        {
            return m_abstraction.template get_adjacent_transition_indices<BackwardTraversal>(state);
        }
        size_t get_num_transitions() const override { return m_abstraction.get_num_transitions(); }

        /* Distances */
        const DistanceList& get_goal_distances() const override { return m_abstraction.get_goal_distances(); }
        Distance get_goal_distance(StateIndex state) const override { return m_abstraction.get_goal_distance(state); };

        // The Prototype Design Pattern
        std::unique_ptr<AbstractionConcept> clone() const override { return std::make_unique<AbstractionModel<A>>(*this); }
    };

    // The Bridge Design Pattern
    std::unique_ptr<AbstractionConcept> m_pimpl;

public:
    template<IsAbstraction A>
    requires IsStaticGraph<typename A::GraphType>
    explicit Abstraction(A abstraction) : m_pimpl(std::make_unique<AbstractionModel<A>>(std::move(abstraction))) {}

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
    const std::shared_ptr<IApplicableActionGenerator>& get_aag() const { return m_pimpl->get_aag(); }
    const std::shared_ptr<StateRepository>& get_ssg() const { return m_pimpl->get_ssg(); }

    /* States */
    StateIndex get_initial_state() const { return m_pimpl->get_initial_state(); }
    const StateIndexSet& get_goal_states() const { return m_pimpl->get_goal_states(); }
    const StateIndexSet& get_deadend_states() const { return m_pimpl->get_deadend_states(); }
    template<IsTraversalDirection Direction>
    std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, Direction>> get_adjacent_state_indices(StateIndex state) const
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
    const GroundActionsEdgeList& get_transitions() const { return m_pimpl->get_transitions(); }
    TransitionCost get_transition_cost(TransitionIndex transition) const { return m_pimpl->get_transition_cost(transition); }
    template<IsTraversalDirection Direction>
    std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, Direction>> get_adjacent_transitions(StateIndex state) const
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
    std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, Direction>> get_adjacent_transition_indices(StateIndex state) const
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
    const DistanceList& get_goal_distances() const { return m_pimpl->get_goal_distances(); }
    Distance get_goal_distance(StateIndex state) const { return m_pimpl->get_goal_distance(state); };
};

static_assert(IsAbstraction<Abstraction>);

using AbstractionList = std::vector<Abstraction>;

}

#endif