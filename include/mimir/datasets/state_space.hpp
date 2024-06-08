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

#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <cstddef>
#include <loki/loki.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{

class Transition
{
private:
    State m_successor_state;
    GroundAction m_creating_action;

public:
    Transition(State successor_state, GroundAction creating_action);

    [[nodiscard]] bool operator==(const Transition& other) const;
    [[nodiscard]] size_t hash() const;

    State get_successor_state() const;
    GroundAction get_creating_action() const;
};

using Transitions = std::vector<Transition>;

/// @brief A StateSpace encapsulates the complete dynamics of a PDDL problem.
/// To keep the memory consumption small, we do not store information dependent on the initial state.
class StateSpaceImpl
{
private:
    PDDLParser m_parser;
    std::shared_ptr<GroundedAAG> m_aag;
    std::shared_ptr<SSG> m_ssg;

    StateList m_states;
    State m_initial_state;

    size_t m_num_transitions;
    std::vector<Transitions> m_forward_transitions;
    std::vector<Transitions> m_backward_transitions;

    std::vector<int> m_goal_distances;

    StateSet m_goal_states;
    StateSet m_deadend_states;

    StateMap<size_t> m_state_indices;
    std::unordered_map<int, StateList> m_states_by_goal_distance;

    /// @brief Constructs a state state from data.
    /// The create function calls this constructor and ensures that
    /// the state space is in a legal state allowing other parts of
    /// the code base to operate on the invariants in the implementation.
    StateSpaceImpl(PDDLParser parser,
                   std::shared_ptr<GroundedAAG> aag,
                   std::shared_ptr<SSG> ssg,
                   StateList states,
                   State initial_state,
                   size_t num_transitions,
                   std::vector<Transitions> forward_transitions,
                   std::vector<Transitions> backward_transitions,
                   std::vector<int> goal_distances,
                   StateSet goal_states,
                   StateSet deadend_states);

public:
    /// @brief Try to create a StateSpace from the given input files with the given resource limits
    /// @param domain_file_path The PDDL domain file.
    /// @param problem_file_path The PDDL problem file.
    /// @param max_num_states The maximum number of states allowed
    /// @param timeout_ms The maximum time spent on creating the StateSpace
    /// @return StateSpace if construction is withing given resource limits, and otherwise nullptr.
    static std::shared_ptr<StateSpaceImpl>
    create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms);

    /* Extended functionality */

    /// @brief Compute shortest distances from the given states computed using BrFS.
    /// @param states A list of states from which shortest distances are computed.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<int> compute_shortest_distances_from_states(const StateList& states, bool forward = true) const;

    /// @brief Compute pairwise state distances using Floyd-Warshall.
    /// @param forward If true, forward transitions are used, and otherwise, backward transitions
    std::vector<std::vector<int>> compute_pairwise_shortest_state_distances(bool forward = true) const;

    /* Getters */
    Problem get_problem() const;

    const StateList& get_states() const;

    State get_initial_state() const;

    const std::vector<Transitions>& get_forward_transitions() const;

    const std::vector<Transitions>& get_backward_transitions() const;

    const std::vector<int>& get_goal_distances() const;

    int get_goal_distance(const State& state) const;

    int get_max_goal_distance() const;

    const StateSet& get_goal_states() const;

    const StateSet& get_deadend_states() const;

    size_t get_num_states() const;

    size_t get_num_transitions() const;

    size_t get_num_goal_states() const;

    size_t get_num_deadend_states() const;

    bool is_deadend_state(const State& state) const;

    State sample_state_with_goal_distance(int goal_distance) const;

    std::shared_ptr<GroundedAAG> get_aag() const;

    std::shared_ptr<SuccessorStateGenerator> get_ssg() const;

    const PDDLParser& get_pddl_parser() const;

    PDDLFactories& get_factories();
};

using StateSpace = std::shared_ptr<StateSpaceImpl>;

}

#endif
