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

#include "mimir/search/actions.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/successor_state_generators.hpp"

#include <loki/loki.hpp>

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
    std::shared_ptr<SuccessorStateGenerator> m_ssg;

    StateList m_states;
    State m_initial_state;

    size_t m_num_transitions;
    std::vector<Transitions> m_forward_transitions;
    std::vector<Transitions> m_backward_transitions;

    std::vector<double> m_goal_distances;

    StateSet m_goal_states;
    StateSet m_deadend_states;

    StateSpaceImpl(PDDLParser parser,
                   std::shared_ptr<GroundedAAG> aag,
                   std::shared_ptr<SuccessorStateGenerator> ssg,
                   StateList states,
                   State initial_state,
                   size_t num_transitions,
                   std::vector<Transitions> forward_transitions,
                   std::vector<Transitions> backward_transitions,
                   std::vector<double> goal_distances,
                   StateSet goal_states,
                   StateSet deadend_states);

public:
    /// @brief Try to create a StateSpace from the given input files with the given resource limits
    /// @param domain_file_path The PDDL domain file.
    /// @param problem_file_path The PDDL problem file.
    /// @param max_num_states The maximum number of states allowed
    /// @param timeout_ms The maximum time spent on creating the StateSpace
    /// @return StateSpace if construction is withing given resource limits, and otherwise nullptr.
    static std::shared_ptr<const StateSpaceImpl>
    create(const fs::path& domain_file_path, const fs::path& problem_file_path, const size_t max_num_states, const size_t timeout_ms);

    /* Extended functionality */

    /// @brief Compute distances from the given state computed using DFS.
    std::vector<double> compute_distances_from_state(const State state) const;

    /// @brief Compute pairwise state distances using Floyd-Warshall.
    std::vector<std::vector<double>> compute_pairwise_state_distances() const;

    /* Getters */
    const StateList& get_states() const;

    State get_initial_state() const;

    const std::vector<Transitions>& get_forward_transitions() const;

    const std::vector<Transitions>& get_backward_transitions() const;

    const std::vector<double>& get_goal_distances() const;

    const StateSet& get_goal_states() const;

    const StateSet& get_deadend_states() const;

    size_t get_num_states() const;

    size_t get_num_transitions() const;

    size_t get_num_goal_states() const;

    size_t get_num_deadend_states() const;
};

using StateSpace = std::shared_ptr<const StateSpaceImpl>;

}

#endif
