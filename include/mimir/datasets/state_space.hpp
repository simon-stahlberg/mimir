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

#include "mimir/common/types.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/state_space/options.hpp"
#include "mimir/datasets/state_space/problem_graph.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::datasets
{
/**
 * StateSpaceImpl
 */

/// @brief `StateSpaceImpl` encapsulates a the state transition model of a problems.
///
/// The central features of this implementation are:
/// - the direct applicability of a large collection of Boost BGL graph algorithms on these graphs, making it convenient to analyze state transition models.
/// - the options to apply symmetry reduction (using graph isomorphism).
class StateSpaceImpl
{
private:
    bool m_is_symmetry_reduced;

    search::SearchContext m_context;

    graphs::ProblemGraph m_graph;

    Index m_initial_vertex;
    IndexSet m_goal_vertices;
    IndexSet m_unsolvable_vertices;

public:
    using Options = state_space::Options;

    StateSpaceImpl(bool is_symmetry_reduced,
                   search::SearchContext context,
                   graphs::ProblemGraph graph,
                   Index initial_vertex,
                   IndexSet goal_vertices,
                   IndexSet unsolvable_vertices);

    /**
     * Constructors
     */

    /// @brief Create a `StateSpace` from the given `search::SearchContext` and the given `Options`.
    /// @param context is the `search::SearchContext`.
    /// @param options are the `Options`
    /// @return Optionally returns the StateSpace upon successful expansion paired with optionally returing canonical state representations when symmetry
    /// reduction is enabled.
    static std::optional<std::pair<StateSpace, std::optional<CertificateMaps>>> create(search::SearchContext context, const Options& options = Options());

    static std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>> create(search::GeneralizedSearchContext contexts,
                                                                                     const Options& options = Options());

    /**
     * Getters
     */

    /// @brief Return true if and only if the symmetry reduction was applied.
    /// @return true if and only if symmetry reduction was applied, and false otherwise.
    bool is_symmetry_reduced() const;

    /// @brief Get the underlying `search::SearchContext`.
    /// @return the underlying `search::SearchContext`.
    const search::SearchContext& get_search_context() const;

    /// @brief Get the underlying `graphs::ProblemGraph`
    /// @return the underlying `graphs::ProblemGraph`.
    const graphs::ProblemGraph& get_graph() const;

    /// @brief Get the `mimir::Index` of the `graphs::ProblemVertex` that is marked as initial vertices.
    /// @return an `mimir::Index` that identifies the initial `graphs::ProblemVertex`.
    Index get_initial_vertex() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ProblemVertex` that are marked as goal vertices.
    /// @return an `mimir::IndexSet` that identifies the set of goal `graphs::ProblemVertex`.
    const IndexSet& get_goal_vertices() const;
    /// @brief Get the `mimir::IndexSet` of the `graphs::ProblemVertex` that are marked as unsolvable vertices.
    /// @return an `mimir::IndexSet` that identifies the set of unsolvable `graphs::ProblemVertex`.
    const IndexSet& get_unsolvable_vertices() const;
};

}

#endif
