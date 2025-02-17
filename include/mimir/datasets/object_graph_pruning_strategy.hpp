/*
 * Copyright (C) 2023 Dominik Drexler and Till Hofmann
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

#ifndef MIMIR_GRAPHS_OBJECT_GRAPH_PRUNING_STRATEGY_HPP_
#define MIMIR_GRAPHS_OBJECT_GRAPH_PRUNING_STRATEGY_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/datasets/problem_class_graph.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/search/state.hpp"

#include <ostream>

namespace mimir
{

enum class ObjectGraphPruningStrategyEnum
{
    None,
    StaticScc,
};

/**
 * ObjectGraphPruningStrategy
 */

/// @brief `ObjectGraphPruningStrategy` is a strategy for pruning information
/// during the construction of an object graph. The default implementation prunes nothing.
class ObjectGraphPruningStrategy
{
public:
    virtual ~ObjectGraphPruningStrategy() = default;

    virtual bool prune(Index, Object) const { return false; };
    virtual bool prune(Index, GroundAtom<Static>) const { return false; };
    virtual bool prune(Index, GroundAtom<Fluent>) const { return false; };
    virtual bool prune(Index, GroundAtom<Derived>) const { return false; };
    virtual bool prune(Index, GroundLiteral<Static>) const { return false; }
    virtual bool prune(Index, GroundLiteral<Fluent>) const { return false; }
    virtual bool prune(Index, GroundLiteral<Derived>) const { return false; }
};

}

#endif
