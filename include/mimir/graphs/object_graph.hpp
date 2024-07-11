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

#include "mimir/formalism/formalism.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/state.hpp"

#include <ostream>

namespace mimir
{

/**
 * ObjectGraphPruningStrategy
 */

class ObjectGraphPruningStrategy
{
public:
    virtual ~ObjectGraphPruningStrategy() = default;

    virtual bool prune(const Object&) const { return false; };
    virtual bool prune(const GroundAtom<Static>) const { return false; };
    virtual bool prune(const GroundAtom<Fluent>) const { return false; };
    virtual bool prune(const GroundAtom<Derived>) const { return false; };
    virtual bool prune(const GroundLiteral<Static>) const { return false; }
    virtual bool prune(const GroundLiteral<Fluent>) const { return false; }
    virtual bool prune(const GroundLiteral<Derived>) const { return false; }
};

class ObjectGraphStaticPruningStrategy : public ObjectGraphPruningStrategy
{
public:
    ObjectGraphStaticPruningStrategy(FlatBitsetBuilder<> pruned_objects = FlatBitsetBuilder<>(),
                                     FlatBitsetBuilder<Static> pruned_ground_atoms = FlatBitsetBuilder<Static>(),
                                     FlatBitsetBuilder<Fluent> pruned_fluent_ground_atoms = FlatBitsetBuilder<Fluent>(),
                                     FlatBitsetBuilder<Derived> pruned_derived_ground_atoms = FlatBitsetBuilder<Derived>()) :
        m_pruned_objects(std::move(pruned_objects)),
        m_pruned_ground_atoms(std::move(pruned_ground_atoms)),
        m_pruned_fluent_ground_atoms(std::move(pruned_fluent_ground_atoms)),
        m_pruned_derived_ground_atoms(std::move(pruned_derived_ground_atoms))
    {
    }

    bool prune(const Object& object) const override { return m_pruned_objects.get(object->get_identifier()); }
    bool prune(const GroundAtom<Static> atom) const override { return m_pruned_ground_atoms.get(atom->get_identifier()); }
    bool prune(const GroundAtom<Fluent> atom) const override { return m_pruned_fluent_ground_atoms.get(atom->get_identifier()); }
    bool prune(const GroundAtom<Derived> atom) const override { return m_pruned_derived_ground_atoms.get(atom->get_identifier()); }
    bool prune(const GroundLiteral<Static> literal) const override { return m_pruned_ground_literals.get(literal->get_atom()->get_identifier()); }
    bool prune(const GroundLiteral<Fluent> literal) const override { return m_pruned_fluent_ground_literals.get(literal->get_atom()->get_identifier()); }
    bool prune(const GroundLiteral<Derived> literal) const override { return m_pruned_derived_ground_literals.get(literal->get_atom()->get_identifier()); }

    ObjectGraphStaticPruningStrategy& operator&=(const ObjectGraphStaticPruningStrategy& other)
    {
        m_pruned_objects &= other.m_pruned_objects;
        m_pruned_ground_atoms &= other.m_pruned_ground_atoms;
        m_pruned_fluent_ground_atoms &= other.m_pruned_fluent_ground_atoms;
        m_pruned_derived_ground_atoms &= other.m_pruned_derived_ground_atoms;
        m_pruned_ground_literals &= other.m_pruned_ground_literals;
        m_pruned_fluent_ground_literals &= other.m_pruned_fluent_ground_literals;
        m_pruned_derived_ground_literals &= other.m_pruned_derived_ground_literals;
        return *this;
    }

private:
    FlatBitsetBuilder<> m_pruned_objects;
    FlatBitsetBuilder<Static> m_pruned_ground_atoms;
    FlatBitsetBuilder<Fluent> m_pruned_fluent_ground_atoms;
    FlatBitsetBuilder<Derived> m_pruned_derived_ground_atoms;
    FlatBitsetBuilder<Static> m_pruned_ground_literals;
    FlatBitsetBuilder<Fluent> m_pruned_fluent_ground_literals;
    FlatBitsetBuilder<Derived> m_pruned_derived_ground_literals;
};

/**
 * ObjectGraph
 */

extern VertexColoredDigraph create_object_graph(const ProblemColorFunction& color_function,
                                                const PDDLFactories& pddl_factories,
                                                Problem problem,
                                                State state,
                                                bool mark_true_goal_literals = false,
                                                const ObjectGraphPruningStrategy& pruning_strategy = ObjectGraphPruningStrategy());

}

#endif
