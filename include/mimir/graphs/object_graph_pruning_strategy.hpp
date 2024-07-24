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

#ifndef MIMIR_GRAPHS_OBJECT_GRAPH_PRUNING_STRATEGY_HPP_
#define MIMIR_GRAPHS_OBJECT_GRAPH_PRUNING_STRATEGY_HPP_

#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/graphs/digraph_vertex_colored.hpp"
#include "mimir/search/flat_types.hpp"
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
    ObjectGraphStaticSccPruningStrategy(size_t num_components, std::vector<SccPruningComponent> pruning_components, std::map<StateIndex, size_t> component_map);

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
        FlatBitsetBuilder<Static> m_pruned_static_goal_literal;
        FlatBitsetBuilder<Fluent> m_pruned_fluent_goal_literal;
        FlatBitsetBuilder<Derived> m_pruned_derived_goal_literal;

        SccPruningComponent& operator&=(const SccPruningComponent& other);

        template<PredicateCategory P>
        const FlatBitsetBuilder<P>& get_pruned_goal_literals() const;
    };

    static std::optional<ObjectGraphStaticSccPruningStrategy> create(Problem problem,
                                                                     std::shared_ptr<PDDLFactories> factories,
                                                                     std::shared_ptr<IAAG> aag,
                                                                     std::shared_ptr<SuccessorStateGenerator> ssg,
                                                                     const StateSpaceOptions& options = StateSpaceOptions());

    size_t get_num_components() const;
    const std::vector<SccPruningComponent>& get_pruning_components() const;
    const std::map<StateIndex, size_t>& get_component_map() const;

private:
    size_t m_num_components;
    std::vector<SccPruningComponent> m_pruning_components;
    std::map<StateIndex, size_t> m_component_map;
};

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out,
                                const std::tuple<const ObjectGraphStaticSccPruningStrategy::SccPruningComponent&, Problem, const PDDLFactories&>& data);

}

#endif
