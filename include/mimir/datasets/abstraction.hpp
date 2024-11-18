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
concept IsAbstraction = requires(T a, State state)
{
    {
        a.get_vertex_index(state)
        } -> std::same_as<Index>;
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
        virtual Index get_vertex_index(State state) const = 0;

        /* Meta data */
        virtual Problem get_problem() const = 0;

        /* Memory */
        virtual const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const = 0;
        virtual const std::shared_ptr<IApplicableActionGenerator>& get_applicable_action_generator() const = 0;
        virtual const std::shared_ptr<StateRepository>& get_state_repository() const = 0;

        /* States */
        virtual Index get_initial_vertex_index() const = 0;
        virtual const IndexSet& get_goal_vertex_indices() const = 0;
        virtual const IndexSet& get_deadend_vertex_indices() const = 0;
        virtual size_t get_num_vertices() const = 0;
        virtual size_t get_num_goal_vertices() const = 0;
        virtual size_t get_num_deadend_vertices() const = 0;
        virtual std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_vertex_indices(Index vertex) const = 0;
        virtual std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_vertex_indices(Index vertex) const = 0;
        virtual bool is_goal_vertex(Index vertex) const = 0;
        virtual bool is_deadend_vertex(Index vertex) const = 0;
        virtual bool is_alive_vertex(Index vertex) const = 0;

        /* Transitions */
        virtual const GroundActionsEdgeList& get_edges() const = 0;
        virtual ContinuousCost get_edge_cost(Index edge) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, ForwardTraversal>> get_forward_adjacent_edges(Index vertex) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_edges(Index vertex) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_edge_indices(Index vertex) const = 0;
        virtual std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_edge_indices(Index vertex) const = 0;
        virtual size_t get_num_edges() const = 0;

        /* Distances */
        virtual const ContinuousCostList& get_goal_distances() const = 0;
        virtual ContinuousCost get_goal_distance(Index vertex) const = 0;

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
        Index get_vertex_index(State state) const override { return m_abstraction.get_vertex_index(state); }

        /* Meta data */
        Problem get_problem() const override { return m_abstraction.get_problem(); }

        /* Memory */
        const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const override { return m_abstraction.get_pddl_repositories(); }
        const std::shared_ptr<IApplicableActionGenerator>& get_applicable_action_generator() const override
        {
            return m_abstraction.get_applicable_action_generator();
        }
        const std::shared_ptr<StateRepository>& get_state_repository() const override { return m_abstraction.get_state_repository(); }

        /* States */
        Index get_initial_vertex_index() const override { return m_abstraction.get_initial_vertex_index(); }
        const IndexSet& get_goal_vertex_indices() const override { return m_abstraction.get_goal_vertex_indices(); }
        const IndexSet& get_deadend_vertex_indices() const override { return m_abstraction.get_deadend_vertex_indices(); }
        std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_vertex_indices(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_vertex_indices<ForwardTraversal>(vertex);
        }
        std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_vertex_indices(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_vertex_indices<BackwardTraversal>(vertex);
        }
        size_t get_num_vertices() const override { return m_abstraction.get_num_vertices(); }
        size_t get_num_goal_vertices() const override { return m_abstraction.get_num_goal_vertices(); }
        size_t get_num_deadend_vertices() const override { return m_abstraction.get_num_deadend_vertices(); }
        bool is_goal_vertex(Index vertex) const override { return m_abstraction.is_goal_vertex(vertex); }
        bool is_deadend_vertex(Index vertex) const override { return m_abstraction.is_deadend_vertex(vertex); }
        bool is_alive_vertex(Index vertex) const override { return m_abstraction.is_alive_vertex(vertex); }

        /* Transitions */
        const GroundActionsEdgeList& get_edges() const override { return m_abstraction.get_edges(); }
        ContinuousCost get_edge_cost(Index edge) const override { return m_abstraction.get_edge_cost(edge); }
        std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, ForwardTraversal>> get_forward_adjacent_edges(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_edges<ForwardTraversal>(vertex);
        }
        std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, BackwardTraversal>> get_backward_adjacent_edges(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_edges<BackwardTraversal>(vertex);
        }
        std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, ForwardTraversal>>
        get_forward_adjacent_edge_indices(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_edge_indices<ForwardTraversal>(vertex);
        }
        std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, BackwardTraversal>>
        get_backward_adjacent_edge_indices(Index vertex) const override
        {
            return m_abstraction.template get_adjacent_edge_indices<BackwardTraversal>(vertex);
        }
        size_t get_num_edges() const override { return m_abstraction.get_num_edges(); }

        /* Distances */
        const ContinuousCostList& get_goal_distances() const override { return m_abstraction.get_goal_distances(); }
        ContinuousCost get_goal_distance(Index vertex) const override { return m_abstraction.get_goal_distance(vertex); };

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
    Index get_vertex_index(State state) const { return m_pimpl->get_vertex_index(state); }

    /* Meta data */
    Problem get_problem() const { return m_pimpl->get_problem(); }

    /* Memory */
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const { return m_pimpl->get_pddl_repositories(); }
    const std::shared_ptr<IApplicableActionGenerator>& get_applicable_action_generator() const { return m_pimpl->get_applicable_action_generator(); }
    const std::shared_ptr<StateRepository>& get_state_repository() const { return m_pimpl->get_state_repository(); }

    /* States */
    Index get_initial_vertex_index() const { return m_pimpl->get_initial_vertex_index(); }
    const IndexSet& get_goal_vertex_indices() const { return m_pimpl->get_goal_vertex_indices(); }
    const IndexSet& get_deadend_vertex_indices() const { return m_pimpl->get_deadend_vertex_indices(); }
    template<IsTraversalDirection Direction>
    std::ranges::subrange<StaticAdjacentVertexIndexConstIterator<GroundActionsEdge, Direction>> get_adjacent_vertex_indices(Index vertex) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_vertex_indices(vertex);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_vertex_indices(vertex);
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "Abstraction::get_adjacent_vertex_indices(...): Missing implementation for IsTraversalDirection.");
        }
    }
    size_t get_num_vertices() const { return m_pimpl->get_num_vertices(); }
    size_t get_num_goal_vertices() const { return m_pimpl->get_num_goal_vertices(); }
    size_t get_num_deadend_vertices() const { return m_pimpl->get_num_deadend_vertices(); }
    bool is_goal_vertex(Index vertex) const { return m_pimpl->is_goal_vertex(vertex); }
    bool is_deadend_vertex(Index vertex) const { return m_pimpl->is_deadend_vertex(vertex); }
    bool is_alive_vertex(Index vertex) const { return m_pimpl->is_alive_vertex(vertex); }

    /* Transitions */
    const GroundActionsEdgeList& get_edges() const { return m_pimpl->get_edges(); }
    ContinuousCost get_edge_cost(Index edge) const { return m_pimpl->get_edge_cost(edge); }
    template<IsTraversalDirection Direction>
    std::ranges::subrange<StaticAdjacentEdgeConstIterator<GroundActionsEdge, Direction>> get_adjacent_edges(Index vertex) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_edges(vertex);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_edges(vertex);
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "Abstraction::get_adjacent_edges(...): Missing implementation for IsTraversalDirection.");
        }
    }
    template<IsTraversalDirection Direction>
    std::ranges::subrange<StaticAdjacentEdgeIndexConstIterator<GroundActionsEdge, Direction>> get_adjacent_edge_indices(Index vertex) const
    {
        if constexpr (std::is_same_v<Direction, ForwardTraversal>)
        {
            return m_pimpl->get_forward_adjacent_edge_indices(vertex);
        }
        else if constexpr (std::is_same_v<Direction, BackwardTraversal>)
        {
            return m_pimpl->get_backward_adjacent_edge_indices(vertex);
        }
        else
        {
            static_assert(dependent_false<Direction>::value, "Abstraction::get_adjacent_edge_indices(...): Missing implementation for IsTraversalDirection.");
        }
    }
    size_t get_num_edges() const { return m_pimpl->get_num_edges(); }

    /* Distances */
    const ContinuousCostList& get_goal_distances() const { return m_pimpl->get_goal_distances(); }
    ContinuousCost get_goal_distance(Index state) const { return m_pimpl->get_goal_distance(state); };
};

static_assert(IsAbstraction<Abstraction>);

using AbstractionList = std::vector<Abstraction>;

}

#endif