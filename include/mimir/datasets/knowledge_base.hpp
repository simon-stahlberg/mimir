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

#ifndef MIMIR_DATASETS_KNOWLEDGE_BASE_HPP_
#define MIMIR_DATASETS_KNOWLEDGE_BASE_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/tuple_graph.hpp"
#include "mimir/search/generalized_search_context.hpp"

namespace mimir::datasets
{

/// @brief `KnowledgeBaseImpl` encapsulates information obtained from a class of planning problems.
class KnowledgeBaseImpl
{
private:
    formalism::Domain m_domain;  ///< The common domain.

    StateSpaceList m_state_spaces;  ///< The state transition models of each `Problem` with optional symmetry reduction.

    std::optional<GeneralizedStateSpace>
        m_generalized_state_space;  ///< Optional abstraction on top of `StateSpaceList` with optional symmetry reduction between problems.

    std::optional<std::vector<TupleGraphList>> m_tuple_graphs;  ///< Optional tuple graphs for each `StateSpace`.

public:
    KnowledgeBaseImpl(formalism::Domain domain,
                      StateSpaceList state_spaces,
                      std::optional<GeneralizedStateSpace> generalized_state_space,
                      std::optional<std::vector<TupleGraphList>> tuple_graphs);

    /**
     * Options
     */

    struct Options
    {
        StateSpaceImpl::Options state_space_options;

        std::optional<GeneralizedStateSpaceImpl::Options> generalized_state_space_options;

        std::optional<TupleGraphImpl::Options> tuple_graph_options;

        Options() : state_space_options(), tuple_graph_options(std::nullopt) {}
        Options(const StateSpaceImpl::Options& state_space_options,
                const std::optional<GeneralizedStateSpaceImpl::Options>& generalized_state_space_options = std::nullopt,
                const std::optional<TupleGraphImpl::Options>& tuple_graph_options = std::nullopt) :
            state_space_options(state_space_options),
            generalized_state_space_options(generalized_state_space_options),
            tuple_graph_options(tuple_graph_options)
        {
        }
    };

    /**
     * Constructors
     */

    static KnowledgeBase create(search::GeneralizedSearchContext contexts, const Options& options = Options());

    /**
     * Getters
     */

    const formalism::Domain& get_domain() const;
    const StateSpaceList& get_state_spaces() const;
    const std::optional<GeneralizedStateSpace>& get_generalized_state_space() const;
    const std::optional<std::vector<TupleGraphList>>& get_tuple_graphs() const;
};
}

#endif