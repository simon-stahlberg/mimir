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

#ifndef MIMIR_DATASETS_GLOBAL_FAITHFUL_ABSTRACTION_HPP_
#define MIMIR_DATASETS_GLOBAL_FAITHFUL_ABSTRACTION_HPP_

#include "mimir/datasets/abstraction.hpp"
#include "mimir/datasets/faithful_abstraction.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

#include <loki/details/utils/filesystem.hpp>
#include <memory>
#include <optional>
#include <ranges>
#include <thread>
#include <unordered_set>
#include <vector>

namespace mimir
{

/// @brief `GlobalFaithfulAbstractState` encapsulates data to access
/// the representative abstract vertex of a faithful abstraction.
class GlobalFaithfulAbstractState
{
private:
    // The index within a `GlobalFaithfulAbstraction`.
    Index m_vertex_index;
    // The index within a `GlobalFaithfulAbstractionList`.
    Index m_global_index;
    // The indices to access the corresponding `FaithfulAbstractStateVertex`.
    Index m_faithful_abstraction_index;
    Index m_faithful_abstraction_vertex_index;

public:
    GlobalFaithfulAbstractState(Index vertex_index, Index global_index, Index faithful_abstraction_index, Index faithful_abstraction_vertex_index);

    bool operator==(const GlobalFaithfulAbstractState& other) const;

    Index get_vertex_index() const;
    Index get_global_index() const;
    Index get_faithful_abstraction_index() const;
    Index get_faithful_abstraction_vertex_index() const;
};

using GlobalFaithfulAbstractStateList = std::vector<GlobalFaithfulAbstractState>;

/// @brief `GlobalFaithfulAbstraction` is a wrapper around a collection of `FaithfulAbstraction`s
/// representing one of the `FaithfulAbstraction` with additional isomorphism reduction applied across the collection.
class GlobalFaithfulAbstraction
{
public:
    using GraphType = typename FaithfulAbstraction::GraphType;

    using VertexIndexConstIteratorType = typename GraphType::VertexIndexConstIteratorType;
    using EdgeIndexConstIteratorType = typename GraphType::EdgeIndexConstIteratorType;

    template<IsTraversalDirection Direction>
    using AdjacentVertexConstIteratorType = typename GraphType::AdjacentVertexConstIteratorType<Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentVertexIndexConstIteratorType = typename GraphType::AdjacentVertexIndexConstIteratorType<Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeConstIteratorType = typename GraphType::AdjacentEdgeConstIteratorType<Direction>;
    template<IsTraversalDirection Direction>
    using AdjacentEdgeIndexConstIteratorType = typename GraphType::AdjacentEdgeIndexConstIteratorType<Direction>;

private:
    GlobalFaithfulAbstraction(bool mark_true_goal_literals,
                              bool use_unit_cost_one,
                              Index index,
                              std::shared_ptr<const FaithfulAbstractionList> abstractions,
                              GlobalFaithfulAbstractStateList states,
                              size_t num_isomorphic_states,
                              size_t num_non_isomorphic_states);

public:
    /// @brief Convenience function when sharing parsers, applicable_action_generators, state_repositorys is not relevant.
    static std::vector<GlobalFaithfulAbstraction> create(const fs::path& domain_filepath,
                                                         const std::vector<fs::path>& problem_filepaths,
                                                         const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    /// @brief Try to create a `GlobalFaithfulAbstractionList` from the given data and the given options.
    /// @param memories External memory to problem, factories, applicable_action_generators, state_repositorys.
    /// @param options the options.
    /// @return `GlobalFaithfulAbstractionList` contains the `GlobalFaithfulAbstraction`s for which the construction was successful.
    static std::vector<GlobalFaithfulAbstraction>
    create(const std::vector<
               std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>& memories,
           const FaithfulAbstractionsOptions& options = FaithfulAbstractionsOptions());

    /**
     * Abstraction functionality
     */

    Index get_vertex_index(State state) const;
    Index get_vertex_index(Index global_index) const;

    /**
     * Extended functionality
     */

    /// @brief Compute the shortest distances from the given vertices using Breadth-first search (unit cost 1) or Dijkstra.
    /// @tparam Direction the direction of traversal.
    /// @param vertices the list of vertices from which shortest distances are computed.
    /// @return the shortest distances from the given vertices to all other vertices.
    template<IsTraversalDirection Direction>
    ContinuousCostList compute_shortest_distances_from_vertices(const IndexList& vertices) const;

    /// @brief Compute pairwise shortest distances using Floyd-Warshall.
    /// @tparam Direction the direction of traversal.
    /// @return the pairwise shortest distances.
    template<IsTraversalDirection Direction>
    ContinuousCostMatrix compute_pairwise_shortest_vertex_distances() const;

    /**
     * Getters
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_mark_true_goal_literals() const;
    bool get_use_unit_cost_one() const;
    Index get_index() const;

    /* Memory */
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
    const std::shared_ptr<IApplicableActionGenerator>& get_applicable_action_generator() const;
    const std::shared_ptr<StateRepository>& get_state_repository() const;
    const FaithfulAbstractionList& get_abstractions() const;

    /* Graph */
    const GraphType& get_graph() const;

    /* States */
    const GlobalFaithfulAbstractStateList& get_vertices() const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(Index vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(Index vertex) const;
    const StateMap<Index>& get_state_to_vertex_index() const;
    const std::unordered_map<Index, Index>& get_global_vertex_index_to_vertex_index() const;
    Index get_initial_vertex_index() const;
    const IndexSet& get_goal_vertex_indices() const;
    const IndexSet& get_deadend_vertex_indices() const;
    size_t get_num_vertices() const;
    size_t get_num_goal_vertices() const;
    size_t get_num_deadend_vertices() const;
    bool is_goal_vertex(Index vertex) const;
    bool is_deadend_vertex(Index vertex) const;
    bool is_alive_vertex(Index vertex) const;
    size_t get_num_isomorphic_states() const;
    size_t get_num_non_isomorphic_states() const;

    /* Transitions */
    const GroundActionsEdgeList& get_edges() const;
    const GroundActionsEdge& get_edge(Index edge) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(Index vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(Index vertex) const;
    ContinuousCost get_edge_cost(Index edge) const;
    size_t get_num_edges() const;

    /* Distances */
    const ContinuousCostList& get_goal_distances() const;
    ContinuousCost get_goal_distance(Index vertex) const;

    /* Additional */
    const std::map<ContinuousCost, IndexList>& get_states_by_goal_distance() const;

private:
    /* Meta data */
    bool m_mark_true_goal_literals;
    bool m_use_unit_cost_one;
    Index m_index;

    /* Memory */
    std::shared_ptr<const FaithfulAbstractionList> m_abstractions;

    /* States */
    GlobalFaithfulAbstractStateList m_states;
    size_t m_num_isomorphic_states;
    size_t m_num_non_isomorphic_states;

    /* Additional */
    std::unordered_map<Index, Index> m_global_vertex_index_to_vertex_index;
};

static_assert(IsAbstraction<GlobalFaithfulAbstraction>);
static_assert(IsStaticGraph<StaticBidirectionalGraph<StaticGraph<FaithfulAbstractStateVertex, GroundActionsEdge>>>);

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const GlobalFaithfulAbstraction& abstraction);

}

template<>
struct std::hash<mimir::GlobalFaithfulAbstractState>
{
    size_t operator()(const mimir::GlobalFaithfulAbstractState& element) const;
};

#endif
