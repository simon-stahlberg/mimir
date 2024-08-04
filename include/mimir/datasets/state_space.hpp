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
#include "mimir/datasets/concrete_state.hpp"
#include "mimir/datasets/concrete_transition.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/graphs/graph.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

#include <cstddef>
#include <loki/loki.hpp>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace mimir
{

struct StateSpaceOptions
{
    bool use_unit_cost_one = true;
    bool remove_if_unsolvable = true;
    uint32_t max_num_states = std::numeric_limits<uint32_t>::max();
    uint32_t timeout_ms = std::numeric_limits<uint32_t>::max();
};

struct StateSpacesOptions
{
    StateSpaceOptions state_space_options = StateSpaceOptions();
    bool sort_ascending_by_num_states = true;
    uint32_t num_threads = std::thread::hardware_concurrency();
};

/// @brief A StateSpace encapsulates the complete dynamics of a PDDL problem.
/// To keep the memory consumption small, we do not store information dependent on the initial state.
class StateSpace
{
private:
    /* Meta data */
    Problem m_problem;
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IAAG> m_aag;
    std::shared_ptr<SuccessorStateGenerator> m_ssg;

    /* States */
    BidirectionalGraph<Graph<ConcreteState, ConcreteTransition>> m_graph;
    StateMap<StateIndex> m_state_to_index;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;

    /* Distances */
    std::vector<Distance> m_goal_distances;

    /* Additional */
    std::map<Distance, StateIndexList> m_states_by_goal_distance;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    StateSpace(Problem problem,
               bool use_unit_cost_one,
               std::shared_ptr<PDDLFactories> pddl_factories,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<SuccessorStateGenerator> ssg,
               BidirectionalGraph<Graph<ConcreteState, ConcreteTransition>> graph,
               StateMap<StateIndex> state_to_index,
               StateIndex initial_state,
               StateIndexSet goal_states,
               StateIndexSet deadend_states,
               std::vector<Distance> goal_distances);

public:
    using StateType = State;
    using TransitionType = ConcreteTransition;

    /// @brief Try to create a StateSpace from the given input files with the given resource limits.
    /// @param problem The problem from which to create the state space.
    /// @param parser External memory to PDDLFactories.
    /// @param aag External memory to aag.
    /// @param ssg External memory to ssg.
    /// @param max_num_states The maximum number of states allowed in the StateSpace.
    /// @param timeout_ms The maximum time spent on creating the StateSpace.
    /// @return StateSpace if construction is within the given resource limits, and otherwise nullptr.
    static std::optional<StateSpace> create(Problem problem,
                                            std::shared_ptr<PDDLFactories> factories,
                                            std::shared_ptr<IAAG> aag,
                                            std::shared_ptr<SuccessorStateGenerator> ssg,
                                            const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::optional<StateSpace>
    create(const fs::path& domain_filepath, const fs::path& problem_filepath, const StateSpaceOptions& options = StateSpaceOptions());

    /// @brief Try to create a StateSpaceList from the given data and the given resource limits.
    /// @param memories External memory to problems, parsers, aags, ssgs.
    /// @param use_unit_cost_one whether to use unit cost one or action costs.
    /// @param remove_if_unsolvable whether to remove state spaces of unsolvable problems.
    /// @param sort_ascending_by_num_states whether the state spaces should be sorted ascending by the number of states.
    /// @param max_num_states The maximum number of states allowed in a StateSpace.
    /// @param timeout_ms The maximum time spent on creating a StateSpace.
    /// @param num_threads The number of threads used for construction.
    /// @return StateSpaceList contains the StateSpaces for which the construction is within the given resource limits.
    static std::vector<StateSpace>
    create(const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
           const StateSpacesOptions& options = StateSpacesOptions());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::vector<StateSpace>
    create(const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const StateSpacesOptions& options = StateSpacesOptions());

    /**
     * Extended functionality
     */

    /// @brief Compute the shortest distances from the given states.
    /// @tparam Direction the direction of traversal.
    /// @param states the list of states from which shortest distances are computed.
    /// @return the shortest distances from the given states to all other states.
    template<IsTraversalDirection Direction>
    std::vector<Distance> compute_shortest_distances_from_states(const StateIndexList& states) const;

    /// @brief Compute pairwise shortest distances using Floyd-Warshall.
    /// @tparam Direction the direction of traversal.
    /// @return the pairwise shortest distances.
    template<IsTraversalDirection Direction>
    std::vector<std::vector<Distance>> compute_pairwise_shortest_state_distances() const;

    /**
     *  Getters
     */

    /* Meta data */
    Problem get_problem() const;
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const;

    /* Graph */
    const BidirectionalGraph<Graph<ConcreteState, ConcreteTransition>>& get_graph() const;

    /* States */
    const ConcreteStateList& get_states() const;
    template<IsTraversalDirection Direction>
    AdjacentVertexIterator<ConcreteState, ConcreteTransition, Direction> get_adjacent_states(StateIndex state) const;
    template<IsTraversalDirection Direction>
    AdjacentVertexIndexIterator<ConcreteTransition, Direction> get_adjacent_state_indices(StateIndex state) const;
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
    const ConcreteTransitionList& get_transitions() const;
    template<IsTraversalDirection Direction>
    AdjacentEdgeIterator<ConcreteTransition, Direction> get_adjacent_transitions(StateIndex state) const;
    template<IsTraversalDirection Direction>
    AdjacentEdgeIndexIterator<ConcreteTransition, Direction> get_adjacent_transition_indices(StateIndex state) const;
    TransitionCost get_transition_cost(TransitionIndex transition) const;
    size_t get_num_transitions() const;

    /* Distances */
    const std::vector<Distance>& get_goal_distances() const;
    Distance get_max_goal_distance() const;

    /* Additional */
    const std::map<Distance, StateIndexList>& get_states_by_goal_distance() const;
    StateIndex sample_state_with_goal_distance(Distance goal_distance) const;
};

static_assert(IsGraph<BidirectionalGraph<Graph<ConcreteState, ConcreteTransition>>>);

using StateSpaceList = std::vector<StateSpace>;

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const StateSpace& state_space);
}

#endif
