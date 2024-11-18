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

#ifndef MIMIR_DATASETS_STATE_SPACE_HPP_
#define MIMIR_DATASETS_STATE_SPACE_HPP_

#include "mimir/common/grouped_vector.hpp"
#include "mimir/datasets/ground_action_edge.hpp"
#include "mimir/datasets/state_vertex.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/graphs/static_graph.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

#include <cstddef>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace mimir
{

/// @brief `StateSpaceOptions` encapsulates options to create a single state space with default parameters.
struct StateSpaceOptions
{
    bool use_unit_cost_one = true;
    bool remove_if_unsolvable = true;
    uint32_t max_num_states = std::numeric_limits<uint32_t>::max();
    uint32_t timeout_ms = std::numeric_limits<uint32_t>::max();
};

/// @brief `StateSpacesOptions` encapsulates options to create a collection of state spaces with default parameters.
struct StateSpacesOptions
{
    StateSpaceOptions state_space_options = StateSpaceOptions();
    bool sort_ascending_by_num_states = true;
    uint32_t num_threads = std::thread::hardware_concurrency();
};

/// @brief `StateSpace` encapsulates the complete dynamics of a PDDL problem.
///
/// The underlying graph type is a `StaticBidirectionalGraph` over `StateVertex` and `GroundActionEdge`.
/// The `StateSpace` stores additional external properties on vertices such as initial state, goal states, deadend states.
/// The getters are simple adapters to follow the notion of states and transitions from the literature.
///
/// Graph algorithms should be applied on the underlying graph, see e.g., `compute_pairwise_shortest_vertex_distances()`.
class StateSpace
{
public:
    using GraphType = StaticBidirectionalGraph<StaticGraph<StateVertex, GroundActionEdge>>;

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
    /// @brief Constructs a `StateSpace` from data.
    /// The create function calls this constructor and ensures that
    /// the `StateSpace` is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    StateSpace(Problem problem,
               bool use_unit_cost_one,
               std::shared_ptr<PDDLRepositories> pddl_repositories,
               std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
               std::shared_ptr<StateRepository> state_repository,
               typename StateSpace::GraphType graph,
               StateMap<Index> state_to_vertex_index,
               Index initial_vertex_index,
               IndexSet goal_vertex_indices,
               IndexSet deadend_vertex_indices,
               ContinuousCostList goal_distances);

public:
    /// @brief Try to create a `StateSpace` from the given input files with the given options.
    /// @param problem The problem from which to create the state space.
    /// @param pddl_repositories External memory to PDDLRepositories.
    /// @param applicable_action_generator External memory to applicable_action_generator.
    /// @param state_repository External memory to state_repository.
    /// @param options the options.
    /// @return StateSpace if construction is within the given options, and otherwise nullptr.
    static std::optional<StateSpace> create(Problem problem,
                                            std::shared_ptr<PDDLRepositories> pddl_repositories,
                                            std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                            std::shared_ptr<StateRepository> state_repository,
                                            const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Convenience function when sharing parsers, applicable_action_generators, state_repositorys is not relevant.
    static std::optional<StateSpace>
    create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Convenience function when sharing parsers, applicable_action_generators, state_repositorys is not relevant.
    static std::vector<StateSpace>
    create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options = StateSpacesOptions());

    /// @brief Try to create a `StateSpaceList` from the given data and the given options.
    /// @param memories External memory to problems, parsers, applicable_action_generators, state_repositorys.
    /// @param options the options.
    /// @return `StateSpaceList` contains the `StateSpace`s for which the construction was successful.
    static std::vector<StateSpace>
    create(const std::vector<
               std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>& memories,
           const StateSpacesOptions& options = StateSpacesOptions());

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
     *  Getters
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
    const std::shared_ptr<IApplicableActionGenerator>& get_applicable_action_generator() const;
    const std::shared_ptr<StateRepository>& get_state_repository() const;

    /* Graph */
    const GraphType& get_graph() const;

    /* States */
    const StateVertexList& get_vertices() const;
    const StateVertex& get_vertex(Index vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_vertices(Index vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_vertex_indices(Index vertex) const;
    Index get_vertex_index(State state) const;
    Index get_initial_vertex_index() const;
    const IndexSet& get_goal_vertex_indices() const;
    const IndexSet& get_deadend_vertex_indices() const;
    size_t get_num_vertices() const;
    size_t get_num_goal_vertices() const;
    size_t get_num_deadend_vertices() const;
    bool is_goal_vertex(Index vertex) const;
    bool is_deadend_vertex(Index vertex) const;
    bool is_alive_vertex(Index vertex) const;

    /* Transitions */
    const GroundActionEdgeList& get_edges() const;
    const GroundActionEdge& get_edge(Index edge) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_edges(Index vertex) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_edge_indices(Index vertex) const;
    ContinuousCost get_edge_cost(Index edge) const;
    size_t get_num_edges() const;

    /* Distances */
    const ContinuousCostList& get_goal_distances() const;
    ContinuousCost get_goal_distance(Index vertex) const;
    ContinuousCost get_max_goal_distance() const;

    /* Additional */
    const std::map<ContinuousCost, IndexList>& get_vertex_indices_by_goal_distance() const;
    Index sample_vertex_index_with_goal_distance(ContinuousCost goal_distance) const;

private:
    /* Meta data */
    Problem m_problem;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;
    std::shared_ptr<IApplicableActionGenerator> m_applicable_action_generator;
    std::shared_ptr<StateRepository> m_state_repository;

    /* States */
    GraphType m_graph;
    StateMap<Index> m_state_to_vertex_index;
    Index m_initial_vertex_index;
    IndexSet m_goal_vertex_indices;
    IndexSet m_deadend_vertex_indices;

    /* Distances */
    ContinuousCostList m_goal_distances;

    /* Additional */
    std::map<ContinuousCost, IndexList> m_vertex_indices_by_goal_distance;
};

using StateSpaceList = std::vector<StateSpace>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const StateSpace& state_space);
}

#endif
