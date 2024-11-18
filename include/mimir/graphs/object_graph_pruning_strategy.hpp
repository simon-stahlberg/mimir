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
#include "mimir/datasets/state_space.hpp"
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

/// @brief `ObjectGraphStaticPruningStrategy` is a strategy for pruning
/// irrelevant static information during the construction of an object graph.
/// Static information is irrelevant if no ground action reachable from
/// the given state ever uses it in a precondition of effect.
class ObjectGraphStaticSccPruningStrategy : public ObjectGraphPruningStrategy
{
public:
    struct SccPruningComponent;
    ObjectGraphStaticSccPruningStrategy(size_t num_components, std::vector<SccPruningComponent> pruning_components, std::vector<size_t> component_map);

    bool prune(Index, Object object) const override;
    bool prune(Index, GroundAtom<Static> atom) const override;
    bool prune(Index, GroundAtom<Fluent> atom) const override;
    bool prune(Index, GroundAtom<Derived> atom) const override;
    bool prune(Index, GroundLiteral<Static> literal) const override;
    bool prune(Index, GroundLiteral<Fluent> literal) const override;
    bool prune(Index, GroundLiteral<Derived> literal) const override;

    struct SccPruningComponent
    {
        FlatBitset m_pruned_objects;
        FlatBitset m_pruned_static_goal_literal;
        FlatBitset m_pruned_fluent_goal_literal;
        FlatBitset m_pruned_derived_goal_literal;

        SccPruningComponent& operator&=(const SccPruningComponent& other);

        template<PredicateTag P>
        const FlatBitset& get_pruned_goal_literals() const;
    };

    static std::optional<ObjectGraphStaticSccPruningStrategy> create(Problem problem,
                                                                     std::shared_ptr<PDDLRepositories> factories,
                                                                     std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                                                     std::shared_ptr<StateRepository> state_repository,
                                                                     const StateSpaceOptions& options = StateSpaceOptions());

    size_t get_num_components() const;
    const std::vector<SccPruningComponent>& get_pruning_components() const;
    const std::vector<size_t>& get_component_map() const;

private:
    size_t m_num_components;
    std::vector<SccPruningComponent> m_pruning_components;
    std::vector<size_t> m_component_map;
};

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out,
                                const std::tuple<const ObjectGraphStaticSccPruningStrategy::SccPruningComponent&, Problem, const PDDLRepositories&>& data);

}

#endif
