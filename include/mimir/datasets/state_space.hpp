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

#include "mimir/datasets/transition_system.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <cstddef>
#include <loki/loki.hpp>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

namespace mimir
{

/// @brief A StateSpace encapsulates the complete dynamics of a PDDL problem.
/// To keep the memory consumption small, we do not store information dependent on the initial state.
class StateSpace
{
private:
    /* Meta data */
    bool m_use_unit_cost_one;

    /* Memory */
    std::shared_ptr<PDDLParser> m_parser;
    std::shared_ptr<IAAG> m_aag;
    std::shared_ptr<ISSG> m_ssg;

    /* States */
    // Note that state.get_id() does not yield the index within the state_space.
    // Use state_space.get_state_index(state) instead.
    StateList m_states;
    StateMap<StateIndex> m_state_to_index;
    StateIndex m_initial_state;
    StateIndexSet m_goal_states;
    StateIndexSet m_deadend_states;

    /* Transitions */
    size_t m_num_transitions;
    std::vector<TransitionList> m_forward_transitions;
    std::vector<TransitionList> m_backward_transitions;

    /* Distances */
    std::vector<double> m_goal_distances;

    /* Additional */
    std::unordered_map<double, StateIndexList> m_states_by_goal_distance;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    StateSpace(bool use_unit_cost_one,
               std::shared_ptr<PDDLParser> parser,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<ISSG> ssg,
               StateList states,
               StateMap<StateIndex> state_to_index,
               StateIndex initial_state,
               StateIndexSet goal_states,
               StateIndexSet deadend_states,
               size_t num_transitions,
               std::vector<TransitionList> forward_transitions,
               std::vector<TransitionList> backward_transitions,
               std::vector<double> goal_distances);

public:
    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::optional<StateSpace> create(const fs::path& domain_filepath,
                                            const fs::path& problem_filepath,
                                            bool use_unit_cost_one = true,
                                            bool remove_if_unsolvable = true,
                                            uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                            uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    /// @brief Try to create a StateSpace from the given input files with the given resource limits.
    /// @param parser External memory to parser.
    /// @param aag External memory to aag.
    /// @param ssg External memory to ssg.
    /// @param max_num_states The maximum number of states allowed in the StateSpace.
    /// @param timeout_ms The maximum time spent on creating the StateSpace.
    /// @return StateSpace if construction is within the given resource limits, and otherwise nullptr.
    static std::optional<StateSpace> create(std::shared_ptr<PDDLParser> parser,
                                            std::shared_ptr<IAAG> aag,
                                            std::shared_ptr<ISSG> ssg,
                                            bool use_unit_cost_one = true,
                                            bool remove_if_unsolvable = true,
                                            uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                            uint32_t timeout_ms = std::numeric_limits<uint32_t>::max());

    /// @brief Convenience function when sharing parsers, aags, ssgs is not relevant.
    static std::vector<StateSpace> create(const fs::path& domain_filepath,
                                          const std::vector<fs::path>& problem_filepaths,
                                          bool use_unit_cost_one = true,
                                          bool remove_if_unsolvable = true,
                                          bool sort_ascending_by_num_states = true,
                                          uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                          uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
                                          uint32_t num_threads = std::thread::hardware_concurrency());

    /// @brief Try to create a StateSpaceList from the given data and the given resource limits.
    /// @param memories External memory to parsers, aags, ssgs.
    /// @param use_unit_cost_one whether to use unit cost one or action costs.
    /// @param remove_if_unsolvable whether to remove state spaces of unsolvable problems.
    /// @param max_num_states The maximum number of states allowed in a StateSpace.
    /// @param timeout_ms The maximum time spent on creating a StateSpace.
    /// @param num_threads The number of threads used for construction.
    /// @return StateSpaceList contains the StateSpaces for which the construction is within the given resource limits.
    static std::vector<StateSpace> create(const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<ISSG>>>& memories,
                                          bool use_unit_cost_one = true,
                                          bool remove_if_unsolvable = true,
                                          bool sort_ascending_by_num_states = true,
                                          uint32_t max_num_states = std::numeric_limits<uint32_t>::max(),
                                          uint32_t timeout_ms = std::numeric_limits<uint32_t>::max(),
                                          uint32_t num_threads = std::thread::hardware_concurrency());

    /**
     * Extended functionality
     */

    /// @brief Compute shortest distances from the given states computed using BrFS.
    /// @param states A list of states from which shortest distances are computed.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<double> compute_shortest_distances_from_states(const StateIndexList& states, bool forward = true) const;

    /// @brief Compute pairwise state distances using Floyd-Warshall.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<std::vector<double>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /**
     *  Getters
     */

    /* Meta data */
    bool get_use_unit_cost_one() const;

    /* Memory */
    const std::shared_ptr<PDDLParser>& get_pddl_parser() const;
    const std::shared_ptr<IAAG>& get_aag() const;
    const std::shared_ptr<ISSG>& get_ssg() const;

    /* States */
    // We cannot ensure that states are having an indexing scheme because
    // users might have created custom states using the successor state generator.
    const StateList& get_states() const;
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
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    /* Distances */
    const std::vector<double>& get_goal_distances() const;
    double get_goal_distance(State state) const;
    double get_max_goal_distance() const;

    /* Additional */
    StateIndex sample_state_with_goal_distance(double goal_distance) const;
};

using StateSpaceList = std::vector<StateSpace>;

/**
 * Static assertions
 */

static_assert(IsTransitionSystem<StateSpace>);

}

#endif
