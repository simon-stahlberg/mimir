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

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/tuple_graph.hpp"
#include "mimir/search/generalized_search_context.hpp"

namespace mimir
{

/// @brief `KnowledgeBase` encapsulates information obtained from a class of planning problems.
class KnowledgeBase
{
private:
    GeneralizedStateSpace m_state_space;  ///< Required core component.

    std::optional<TupleGraphCollection> m_tuple_graphs;  ///< First optional component. Pass TupleGraphCollection::Options to instantiate it!
public:
    /**
     * Options
     */

    struct Options
    {
        GeneralizedStateSpace::Options state_space_options;

        std::optional<TupleGraphCollection::Options> tuple_graph_options;

        Options() : state_space_options(), tuple_graph_options(std::nullopt) {}
        Options(const GeneralizedStateSpace::Options& state_space_options,
                const std::optional<TupleGraphCollection::Options>& tuple_graph_options = std::nullopt) :
            state_space_options(state_space_options),
            tuple_graph_options(tuple_graph_options)
        {
        }
    };

    /**
     * Constructors
     */

    KnowledgeBase(GeneralizedSearchContext contexts, const Options& options = Options());

    static std::unique_ptr<KnowledgeBase> create(GeneralizedSearchContext contexts, const Options& options = Options());

    /**
     * Getters
     */

    const GeneralizedStateSpace& get_generalized_state_space() const;
    const std::optional<TupleGraphCollection>& get_tuple_graphs() const;
};
}

#endif