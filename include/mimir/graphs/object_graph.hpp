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

#ifndef MIMIR_GRAPHS_OBJECT_GRAPH_HPP_
#define MIMIR_GRAPHS_OBJECT_GRAPH_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/state.hpp"

#include <ostream>

namespace mimir
{

/**
 * ObjectGraphPruningStrategy
 */

/// @brief `ObjectGraphPruningStrategy` is a strategy for pruning information
/// during the construction of an object graph. The default implementation prunes nothing.
class ObjectGraphPruningStrategy
{
public:
    virtual ~ObjectGraphPruningStrategy() = default;

    virtual bool prune(StateIndex, Object) const { return false; };
    virtual bool prune(StateIndex, GroundAtom<Static>) const { return false; };
    virtual bool prune(StateIndex, GroundAtom<Fluent>) const { return false; };
    virtual bool prune(StateIndex, GroundAtom<Derived>) const { return false; };
    virtual bool prune(StateIndex, GroundLiteral<Static>) const { return false; }
    virtual bool prune(StateIndex, GroundLiteral<Fluent>) const { return false; }
    virtual bool prune(StateIndex, GroundLiteral<Derived>) const { return false; }
};

/// @brief `ObjectGraphStaticPruningStrategy` is a strategy for pruning
/// irrelevant static information during the construction of an object graph.
/// Static information is irrelevant if no ground action reachable from
/// the given state ever uses it in a precondition of effect.
class ObjectGraphStaticSccPruningStrategy : public ObjectGraphPruningStrategy
{
public:
    struct SccPruningComponent;
    ObjectGraphStaticSccPruningStrategy(std::vector<SccPruningComponent> pruning_components, std::map<StateIndex, size_t> m_component_map);

    bool prune(StateIndex, Object object) const override;
    bool prune(StateIndex, GroundAtom<Static> atom) const override;
    bool prune(StateIndex, GroundAtom<Fluent> atom) const override;
    bool prune(StateIndex, GroundAtom<Derived> atom) const override;
    bool prune(StateIndex, GroundLiteral<Static> literal) const override;
    bool prune(StateIndex, GroundLiteral<Fluent> literal) const override;
    bool prune(StateIndex, GroundLiteral<Derived> literal) const override;

    struct SccPruningComponent
    {
        FlatBitsetBuilder<> m_pruned_objects;
        FlatBitsetBuilder<Static> m_pruned_static_ground_atoms;
        FlatBitsetBuilder<Fluent> m_pruned_fluent_ground_atoms;
        FlatBitsetBuilder<Derived> m_pruned_derived_ground_atoms;
        FlatBitsetBuilder<Static> m_pruned_static_ground_literals;
        FlatBitsetBuilder<Fluent> m_pruned_fluent_ground_literals;
        FlatBitsetBuilder<Derived> m_pruned_derived_ground_literals;

        SccPruningComponent& operator&=(const SccPruningComponent& other);
    };

private:
    std::vector<SccPruningComponent> m_pruning_components;
    std::map<StateIndex, size_t> m_component_map;
};

/**
 * ObjectGraph
 */

/// @brief Create an object graph as a vertex colored graph for a given state.
/// @param color_function is the function used to color the vertices in the object graph.
/// @param pddl_factories is the PDDLFactory.
/// @param problem is the Problem.
/// @param state is the state.
/// @param mark_true_goal_literals represents whether literals that are true should have a special color.
/// @param pruning_strategy is the strategy used for pruning information from the object graph.
/// @return a vertex colored graph that represents the object graph.
extern VertexColoredDigraph create_object_graph(const ProblemColorFunction& color_function,
                                                const PDDLFactories& pddl_factories,
                                                Problem problem,
                                                State state,
                                                StateIndex state_index = 0,
                                                bool mark_true_goal_literals = false,
                                                const ObjectGraphPruningStrategy& pruning_strategy = ObjectGraphPruningStrategy());

}

#endif
