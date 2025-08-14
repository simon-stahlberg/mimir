/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "mimir/datasets/state_space_sampler.hpp"

#include "mimir/common/types.hpp"
#include "mimir/datasets/state_space/problem_graph.hpp"
#include "mimir/graphs/graph_traversal_interface.hpp"
#include "mimir/search/state.hpp"

namespace mimir::datasets
{

StateSpaceSamplerImpl::StateSpaceSamplerImpl(StateSpace state_space) :
    m_state_space(std::move(state_space)),
    m_vertex_index_map(),
    m_vertex_index_map_by_steps(),
    m_dead_end_vertices(),
    m_rng(std::random_device {}()),
    m_max_steps_to_goal(0)
{
    for (const auto& vertex : m_state_space->get_graph().get_vertices())
    {
        const auto state = mimir::graphs::get_state(vertex);
        const auto dead_end = mimir::graphs::is_unsolvable(vertex);
        m_vertex_index_map[state.get_index()] = vertex.get_index();

        if (dead_end)
        {
            m_dead_end_vertices.push_back(vertex.get_index());
        }
        else
        {
            const auto steps = mimir::graphs::get_unit_goal_distance(vertex);
            const auto it = m_vertex_index_map_by_steps.find(steps);
            if (it == m_vertex_index_map_by_steps.end())
            {
                m_vertex_index_map_by_steps[steps] = std::vector<graphs::VertexIndex> { vertex.get_index() };
            }
            else
            {
                it->second.push_back(vertex.get_index());
            }
            m_max_steps_to_goal = std::max(m_max_steps_to_goal, static_cast<size_t>(steps));
        }
    }
}

const StateSpace& StateSpaceSamplerImpl::get_state_space() const { return m_state_space; }

mimir::search::State StateSpaceSamplerImpl::sample_state()
{
    const auto& graph = m_state_space->get_graph();
    const auto num_vertices = graph.get_num_vertices();

    if (num_vertices == 0)
    {
        throw std::runtime_error("Cannot sample state from an empty state space.");
    }

    const auto random_vertex_index = sample_vertex_index(0, num_vertices - 1);
    const auto& random_vertex = graph.get_vertex(random_vertex_index);
    return mimir::graphs::get_state(random_vertex);
}

mimir::search::State StateSpaceSamplerImpl::sample_state_n_steps_from_goal(int n)
{
    if (m_vertex_index_map_by_steps.empty())
    {
        throw std::runtime_error("No states available for sampling.");
    }

    const auto it = m_vertex_index_map_by_steps.find(n);
    if (it == m_vertex_index_map_by_steps.end() || it->second.empty())
    {
        throw std::runtime_error("No states found with the specified number of steps from goal.");
    }

    const auto& vertex_indices = it->second;
    const auto random_index = sample_vertex_index(0, vertex_indices.size() - 1);
    const auto& random_vertex_index = vertex_indices[random_index];

    const auto& graph = m_state_space->get_graph();
    const auto& random_vertex = graph.get_vertex(random_vertex_index);
    return mimir::graphs::get_state(random_vertex);
}

mimir::search::State StateSpaceSamplerImpl::sample_dead_end_state()
{
    if (m_dead_end_vertices.empty())
    {
        throw std::runtime_error("No dead-end states available for sampling.");
    }

    const auto random_index = sample_vertex_index(0, m_dead_end_vertices.size() - 1);
    const auto& random_vertex_index = m_dead_end_vertices[random_index];

    const auto& graph = m_state_space->get_graph();
    const auto& random_vertex = graph.get_vertex(random_vertex_index);
    return mimir::graphs::get_state(random_vertex);
}

mimir::search::State StateSpaceSamplerImpl::get_state(Index index) const
{
    const auto it = m_vertex_index_map.find(index);
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::get_state(vertex);
    }
    throw std::runtime_error("State with index not found in the state space.");
}

std::vector<mimir::search::State> StateSpaceSamplerImpl::get_states() const
{
    std::vector<mimir::search::State> states;
    for (const auto& vertex : m_state_space->get_graph().get_vertices())
    {
        const auto state = mimir::graphs::get_state(vertex);
        states.push_back(state);
    }
    return states;
}

size_t StateSpaceSamplerImpl::get_num_states() const { return m_vertex_index_map.size(); }

size_t StateSpaceSamplerImpl::get_num_dead_end_states() const { return m_dead_end_vertices.size(); }

size_t StateSpaceSamplerImpl::get_num_alive_states() const { return get_num_states() - get_num_dead_end_states(); }

size_t StateSpaceSamplerImpl::get_max_steps_to_goal() const { return m_max_steps_to_goal; }

bool StateSpaceSamplerImpl::is_dead_end_state(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::is_unsolvable(vertex);
    }
    throw std::runtime_error("State not found in the state space.");
}

bool StateSpaceSamplerImpl::is_goal_state(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::is_goal(vertex);
    }
    throw std::runtime_error("State not found in the state space.");
}

bool StateSpaceSamplerImpl::is_initial_state(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::is_initial(vertex);
    }
    throw std::runtime_error("State not found in the state space.");
}

DiscreteCost StateSpaceSamplerImpl::get_steps_to_goal(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::get_unit_goal_distance(vertex);
    }
    throw std::runtime_error("State not found in the state space.");
}

ContinuousCost StateSpaceSamplerImpl::get_cost_to_goal(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        const auto& graph = m_state_space->get_graph();
        const auto& vertex = graph.get_vertex(it->second);
        return mimir::graphs::get_action_goal_distance(vertex);
    }
    throw std::runtime_error("State not found in the state space.");
}

std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>>
StateSpaceSamplerImpl::get_forward_transitions(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        auto transitions = std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>>();
        const auto source_vertex_index = it->second;
        const auto& graph = m_state_space->get_graph();
        const auto forward_edges = graph.get_adjacent_edges<graphs::ForwardTag>(source_vertex_index);
        for (auto& edge : forward_edges)
        {
            const auto target_vertex_index = graph.get_target<graphs::ForwardTag>(edge.get_index());
            const auto& target_vertex = graph.get_vertex(target_vertex_index);
            const auto target_state = mimir::graphs::get_state(target_vertex);
            const auto action = mimir::graphs::get_action(edge);
            transitions.emplace_back(action, target_state);
        }
        return transitions;
    }
    throw std::runtime_error("State not found in the state space.");
}

std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>>
StateSpaceSamplerImpl::get_backward_transitions(const mimir::search::State& state) const
{
    const auto it = m_vertex_index_map.find(state.get_index());
    if (it != m_vertex_index_map.end())
    {
        auto transitions = std::vector<std::pair<mimir::formalism::GroundAction, mimir::search::State>>();
        const auto target_vertex_index = it->second;
        const auto& graph = m_state_space->get_graph();
        const auto backward_edges = graph.get_adjacent_edges<graphs::BackwardTag>(target_vertex_index);
        for (auto& edge : backward_edges)
        {
            const auto source_vertex_index = graph.get_source<graphs::BackwardTag>(edge.get_index());
            const auto& source_vertex = graph.get_vertex(source_vertex_index);
            const auto source_state = mimir::graphs::get_state(source_vertex);
            const auto action = mimir::graphs::get_action(edge);
            transitions.emplace_back(action, source_state);
        }
        return transitions;
    }
    throw std::runtime_error("State not found in the state space.");
}

graphs::VertexIndex StateSpaceSamplerImpl::sample_vertex_index(graphs::VertexIndex min, graphs::VertexIndex max)
{
    std::uniform_int_distribution<graphs::VertexIndex> dist(min, max);
    return dist(m_rng);
}

void StateSpaceSamplerImpl::set_seed(unsigned int seed) { m_rng.seed(seed); }

}
