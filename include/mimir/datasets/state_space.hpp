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
    // Meta data
    fs::path m_domain_filepath;
    fs::path m_problem_filepath;

    // Memory
    std::shared_ptr<PDDLParser> m_parser;
    std::shared_ptr<GroundedAAG> m_aag;
    std::shared_ptr<SSG> m_ssg;

    // States
    StateList m_states;
    StateId m_initial_state;
    StateIdSet m_goal_states;
    StateIdSet m_deadend_states;

    // Transitions
    size_t m_num_transitions;
    std::vector<TransitionList> m_forward_transitions;
    std::vector<TransitionList> m_backward_transitions;

    // Distances
    std::vector<int> m_goal_distances;

    // Additional
    std::unordered_map<int, StateIdList> m_states_by_goal_distance;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    StateSpace(fs::path domain_filepath,
               fs::path problem_filepath,
               std::shared_ptr<PDDLParser> parser,
               std::shared_ptr<GroundedAAG> aag,
               std::shared_ptr<SSG> ssg,
               StateList states,
               StateId initial_state,
               StateIdSet goal_states,
               StateIdSet deadend_states,
               size_t num_transitions,
               std::vector<TransitionList> forward_transitions,
               std::vector<TransitionList> backward_transitions,
               std::vector<int> goal_distances);

public:
    /// @brief Try to create a StateSpace from the given input files with the given resource limits.
    /// @param domain_filepath The PDDL domain file.
    /// @param problem_filepath The PDDL problem file.
    /// @param max_num_states The maximum number of states allowed in the StateSpace.
    /// @param timeout_ms The maximum time spent on creating the StateSpace.
    /// @return StateSpace if construction is within the given resource limits, and otherwise nullptr.
    static std::optional<StateSpace> create(const fs::path& domain_filepath,
                                            const fs::path& problem_filepath,
                                            const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                            const size_t timeout_ms = std::numeric_limits<size_t>::max());

    /// @brief Try to create a StateSpaceList from the given input files with the given resource limits.
    /// @param domain_filepath The PDDL domain file.
    /// @param problem_filepaths The PDDL problem files.
    /// @param max_num_states The maximum number of states allowed in a StateSpace.
    /// @param timeout_ms The maximum time spent on creating a StateSpace.
    /// @param num_threads The number of threads used for construction.
    /// @return StateSpaceList contains the StateSpaces for which the construction is within the given resource limits.
    static std::vector<StateSpace> create(const fs::path& domain_filepath,
                                          const std::vector<fs::path>& problem_filepaths,
                                          const size_t max_num_states = std::numeric_limits<size_t>::max(),
                                          const size_t timeout_ms = std::numeric_limits<size_t>::max(),
                                          const size_t num_threads = std::thread::hardware_concurrency());

    /* Extended functionality */

    /// @brief Compute shortest distances from the given states computed using BrFS.
    /// @param states A list of states from which shortest distances are computed.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<int> compute_shortest_distances_from_states(const StateIdList& states, bool forward = true) const;

    /// @brief Compute pairwise state distances using Floyd-Warshall.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<std::vector<int>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /* Getters */
    // Meta data
    const fs::path& get_domain_filepath() const;
    const fs::path& get_problem_filepath() const;

    // Memory
    const PDDLParser& get_pddl_parser() const;
    const std::shared_ptr<PDDLFactories>& get_pddl_factories() const;
    const std::shared_ptr<GroundedAAG>& get_aag() const;
    const std::shared_ptr<SuccessorStateGenerator>& get_ssg() const;
    Problem get_problem() const;

    // States
    const StateList& get_states() const;
    StateId get_initial_state() const;
    const StateIdSet& get_goal_states() const;
    const StateIdSet& get_deadend_states() const;
    size_t get_num_states() const;
    size_t get_num_goal_states() const;
    size_t get_num_deadend_states() const;
    bool is_deadend_state(const State& state) const;

    // Transitions
    size_t get_num_transitions() const;
    const std::vector<TransitionList>& get_forward_transitions() const;
    const std::vector<TransitionList>& get_backward_transitions() const;

    // Distances
    const std::vector<int>& get_goal_distances() const;
    int get_goal_distance(const State& state) const;
    int get_max_goal_distance() const;

    // Additional
    StateId sample_state_with_goal_distance(int goal_distance) const;
};

using StateSpaceList = std::vector<StateSpace>;

static_assert(IsTransitionSystem<StateSpace>);

}

#endif
