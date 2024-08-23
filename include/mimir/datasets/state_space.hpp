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
#include "mimir/datasets/state_vertex.hpp"
#include "mimir/datasets/ground_action_edge.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/parser.hpp"
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
/// Graph algorithms should be applied on the underlying graph, see e.g., `compute_pairwise_shortest_state_distances()`.
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
               std::shared_ptr<PDDLFactories> pddl_factories,
               std::shared_ptr<IApplicableActionGenerator> aag,
               std::shared_ptr<StateRepository> ssg,
               GraphType graph,
               StateMap<StateIndex> state_to_index,
               StateIndex initial_state,
               StateIndexSet goal_states,
               StateIndexSet deadend_states,
               DistanceList goal_distances);

public:
    /// @brief Try to create a `StateSpace` from the given input files with the given options.
    /// @param problem The problem from which to create the state space.
    /// @param parser External memory to PDDLFactories.
    /// @param aag External memory to aag.
    /// @param ssg External memory to ssg.
    /// @param options the options.
    /// @return StateSpace if construction is within the given options, and otherwise nullptr.
    static std::optional<StateSpace> create(Problem problem,
                                            std::shared_ptr<PDDLFactories> factories,
                                            std::shared_ptr<IApplicableActionGenerator> aag,
                                            std::shared_ptr<StateRepository> ssg,
                                            const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::optional<StateSpace>
    create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::vector<StateSpace>
    create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options = StateSpacesOptions());

    /// @brief Try to create a `StateSpaceList` from the given data and the given options.
    /// @param memories External memory to problems, parsers, aags, ssgs.
    /// @param options the options.
    /// @return `StateSpaceList` contains the `StateSpace`s for which the construction was successful.
    static std::vector<StateSpace> create(
        const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
            memories,
        const StateSpacesOptions& options = StateSpacesOptions());

    /**
     * Extended functionality
     */

    /// @brief Compute the shortest distances from the given states using Breadth-first search (unit cost 1) or Dijkstra.
    /// @tparam Direction the direction of traversal.
    /// @param states the list of states from which shortest distances are computed.
    /// @return the shortest distances from the given states to all other states.
    template<IsTraversalDirection Direction>
    DistanceList compute_shortest_distances_from_states(const StateIndexList& states) const;

    /// @brief Compute pairwise shortest distances using Floyd-Warshall.
    /// @tparam Direction the direction of traversal.
    /// @return the pairwise shortest distances.
    template<IsTraversalDirection Direction>
    DistanceMatrix compute_pairwise_shortest_state_distances() const;

    /**
     *  Getters
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const;
    const std::shared_ptr<IApplicableActionGenerator>& get_aag() const;
    const std::shared_ptr<StateRepository>& get_ssg() const;

    /* Graph */
    const GraphType& get_graph() const;

    /* States */
    const StateVertexList& get_states() const;
    const StateVertex& get_state(StateIndex state) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexConstIteratorType<Direction>> get_adjacent_states(StateIndex state) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentVertexIndexConstIteratorType<Direction>> get_adjacent_state_indices(StateIndex state) const;
    StateIndex get_state_index(State state) const;
    StateIndex get_initial_state() const;
    const StateIndexSet& get_goal_states() const;
    const StateIndexSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    bool is_goal_state(StateIndex state) const;
    bool is_deadend_state(StateIndex state) const;
    bool is_alive_state(StateIndex state) const;

    /* Transitions */
    const GroundActionEdgeList& get_transitions() const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeConstIteratorType<Direction>> get_adjacent_transitions(StateIndex state) const;
    template<IsTraversalDirection Direction>
    std::ranges::subrange<AdjacentEdgeIndexConstIteratorType<Direction>> get_adjacent_transition_indices(StateIndex state) const;
    TransitionCost get_transition_cost(TransitionIndex transition) const;
    size_t get_num_transitions() const;

    /* Distances */
    const DistanceList& get_goal_distances() const;
    Distance get_goal_distance(StateIndex state) const;
    Distance get_max_goal_distance() const;

    /* Additional */
    const std::map<Distance, StateIndexList>& get_states_by_goal_distance() const;
    StateIndex sample_state_with_goal_distance(Distance goal_distance) const;

private:
    /* Meta data */
    Problem m_problem;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IApplicableActionGenerator> m_aag;
    std::shared_ptr<StateRepository> m_ssg;

    /* States */
    GraphType m_graph;
    StateMap<StateIndex> m_state_to_index;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;

    /* Distances */
    DistanceList m_goal_distances;

    /* Additional */
    std::map<Distance, StateIndexList> m_states_by_goal_distance;
};

using StateSpaceList = std::vector<StateSpace>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const StateSpace& state_space);
}

#endif
