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

#ifndef MIMIR_DATASETS_STATE_SPACE_SAMPLER_HPP_
#define MIMIR_DATASETS_STATE_SPACE_SAMPLER_HPP_

#include <cstddef>
#include <random>
#include <unordered_map>

#include "mimir/common/types.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/graphs/types.hpp"

namespace mimir::datasets
{
/**
 * StateSpaceSamplerImpl
 */

/// @brief `StateSpaceSamplerImpl` extends the functionality of `StateSpaceImpl` by providing sampling capabilities for state spaces.
class StateSpaceSamplerImpl
{
private:
    StateSpace m_state_space;
    std::unordered_map<Index, graphs::VertexIndex> m_vertex_index_map;
    std::unordered_map<DiscreteCost, std::vector<graphs::VertexIndex>> m_vertex_index_map_by_steps;
    std::vector<graphs::VertexIndex> m_dead_end_vertices;
    std::mt19937 m_rng;
    size_t m_max_steps_to_goal;

    graphs::VertexIndex sample_vertex_index(graphs::VertexIndex min, graphs::VertexIndex max);

public:
    StateSpaceSamplerImpl(StateSpace state_space);

    mimir::search::State sample_state();

    mimir::search::State sample_state_n_steps_from_goal(int n);

    mimir::search::State sample_dead_end_state();

    const StateSpace& get_state_space() const;

    mimir::search::State get_state(Index index) const;

    std::vector<mimir::search::State> get_states() const;

    size_t get_num_states() const;

    size_t get_num_dead_end_states() const;

    size_t get_num_alive_states() const;

    size_t get_max_steps_to_goal() const;

    bool is_dead_end_state(const mimir::search::State& state) const;

    bool is_goal_state(const mimir::search::State& state) const;

    bool is_initial_state(const mimir::search::State& state) const;

    DiscreteCost get_steps_to_goal(const mimir::search::State& state) const;

    ContinuousCost get_cost_to_goal(const mimir::search::State& state) const;

    std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>> get_forward_transitions(const mimir::search::State& state) const;

    std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>> get_backward_transitions(const mimir::search::State& state) const;

    void set_seed(unsigned int seed);
};

}

#endif
