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

#ifndef MIMIR_DATASETS_GENERALIZED_STATE_SPACE_HPP_
#define MIMIR_DATASETS_GENERALIZED_STATE_SPACE_HPP_

#include "mimir/common/types.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space/class_graph.hpp"
#include "mimir/datasets/generalized_state_space/options.hpp"
#include "mimir/datasets/state_space/problem_graph.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::datasets
{
/**
 * GeneralizedStateSpace
 */

/// @brief `GeneralizedStateSpace` is an additional graph build on top of `StateSpaceList` to represent the state transition model of a class of problems.
///
/// The central features of this implementation are:
/// - the direct applicability of a large collection of Boost BGL graph algorithms on these graphs, making it convenient to analyze state transition models.
/// - the option to apply symmetry reduction (using graph isomorphism).
///
/// The `graphs::ClassGraph` acts as the main interface to retrieve information about the state transition model of a class of problems.
/// Each vertex in the `graphs::ClassGraph` has a single representative from a `graphs::ProblemGraph`.
/// When symmetry reduction is disabled, there is a strict one-to-one mapping between `graphs::ClassVertex` and `graphs::ProblemVertex`
/// and a strict one-to-one mapping between `graphs::ClassEdge` and `graphs::ProblemEdge`.
/// When symmetry reduction is enabled, the mapping may become a strict one-to-many mapping, in which case, the `graphs::ClassVertex`
/// or `graphs::ClassEdge` acts as an interface to access the representative.
///
/// The `GeneralizedStateSpace` provides several convenience functions to map between `graphs::ClassVertex` (resp. `graphs::ClassEdge`) and
/// `graphs::ProblemVertex` (resp. `graphs::ProblemEdge`).
class GeneralizedStateSpaceImpl
{
private:
    StateSpaceList m_state_spaces;

    graphs::ClassGraph m_graph;

    IndexSet m_initial_vertices;
    IndexSet m_goal_vertices;
    IndexSet m_unsolvable_vertices;

    std::unordered_map<const formalism::ProblemImpl*, graphs::VertexIndexList> m_vertex_mappings;
    std::unordered_map<const formalism::ProblemImpl*, graphs::EdgeIndexList> m_edge_mappings;

public:
    using Options = generalized_state_space::Options;

    GeneralizedStateSpaceImpl(StateSpaceList state_spaces,
                              graphs::ClassGraph graph,
                              IndexSet initial_vertices,
                              IndexSet goal_vertices,
                              IndexSet unsolvable_vertices,
                              std::unordered_map<const formalism::ProblemImpl*, graphs::VertexIndexList> vertex_mappings,
                              std::unordered_map<const formalism::ProblemImpl*, graphs::EdgeIndexList> edge_mappings);

    /**
     * Constructors
     */

    /// @brief Construct a `GeneralizedStateSpace` from the given `StateSpaceList` and the `Options`.
    /// Note that the constructor may internally prune irrelevant `StateSpace` if detected by symmetry reduction.
    /// @param state_spaces is the `StateSpaceList`.
    /// @param options are the options used to derive the `GeneralizedStateSpace`.
    static std::pair<GeneralizedStateSpace, std::optional<CertificateMapsList>>
    create(const std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>>& state_spaces, const Options& options);

    /**
     * Getters
     */

    /// @brief Get the underlying `StateSpaceList`
    /// @return the underlying `StateSpaceList`.
    const StateSpaceList& get_state_spaces() const;

    /// @brief Get the underlying `graphs::ClassGraph`
    /// @return the underlying `graphs::ClassGraph`.
    const graphs::ClassGraph& get_graph() const;

    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as initial vertices.
    /// @return an `mimir::IndexSet` that identifies the set of initial `graphs::ClassVertex`.
    const IndexSet& get_initial_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as goal vertices.
    /// @return an `mimir::IndexSet` that identifies the set of goal `graphs::ClassVertex`.
    const IndexSet& get_goal_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ClassVertex` that are marked as unsolvable vertices.
    /// @return an `mimir::IndexSet` that identifies the set of unsolvable `graphs::ClassVertex`.
    const IndexSet& get_unsolvable_vertices() const;

    /// @brief Get the representative `StateSpace` of the given `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ClassVertex`.
    /// @return the representative `StateSpace` of the given `graphs::ClassVertex`.
    const StateSpace& get_state_space(const graphs::ClassVertex& vertex) const;
    /// @brief Get the representative `StateSpace` of the given `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ClassEdge`.
    /// @return the representative `StateSpace` of the given `graphs::ClassEdge`.
    const StateSpace& get_state_space(const graphs::ClassEdge& edge) const;
    /// @brief Get the representative `graphs::ProblemVertex` of the given `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ClassVertex`.
    /// @return the representative `graphs::ProblemVertex` of the given `graphs::ClassVertex`.
    const graphs::ProblemVertex& get_problem_vertex(const graphs::ClassVertex& vertex) const;
    /// @brief Get the representative `graphs::ProblemEdge` of the given `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ClassEdge`.
    /// @return the representative `graphs::ProblemEdge` of the given `graphs::ClassEdge`.
    const graphs::ProblemEdge& get_problem_edge(const graphs::ClassEdge& edge) const;

    /// @brief Lift `graphs::ProblemVertex` back to its corresponding `graphs::ClassVertex`.
    /// Note that other `graphs::ProblemVertex` may map to the same `graphs::ClassVertex`.
    /// @param vertex is the `graphs::ProblemVertex`
    /// @return the corresponding `graphs::ClassVertex`.
    const graphs::ClassVertex& get_class_vertex(const graphs::ProblemVertex& vertex) const;
    /// @brief Lift `graphs::ProblemEdge` back to its corresponding `graphs::ClassEdge`.
    /// Note that other `graphs::ProblemEdge` may map to the same `graphs::ClassEdge`.
    /// @param vertex is the `graphs::ProblemEdge`
    /// @return the corresponding `graphs::ClassEdge`.
    const graphs::ClassEdge& get_class_edge(const graphs::ProblemEdge& edge) const;

    /**
     * Construct subgraphs for learning from fragments of the `ClassGraph`.
     */

    /// @brief Create the induced `ClassGraph` by the given `class_vertex_indices`.
    /// This function copies the set of `graphs::ClassVertex`
    /// and all `graphs::ClassEdge` between those vertices into a new `ClassGraph`.
    std::tuple<graphs::ClassGraph, IndexList, IndexList> create_induced_subgraph_from_class_vertex_indices(const IndexList& class_vertex_indices) const;
    /// @brief Create the induced `ClassGraph` by the given `problem_indices`.
    /// This function copies the corresponding sets of `graphs::ClassVertex`
    /// and `graphs::ClassEdge` into a new `ClassGraph`.
    std::tuple<graphs::ClassGraph, IndexList, IndexList> create_induced_subgraph_from_problem_indices(const IndexList& problem_indices) const;
};
}

#endif
