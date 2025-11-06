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

#ifndef MIMIR_SEARCH_DECLARATIONS_HPP_
#define MIMIR_SEARCH_DECLARATIONS_HPP_

// Do not include headers with transitive dependencies.
#include "mimir/common/declarations.hpp"
#include "mimir/formalism/declarations.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace mimir::search
{

class SearchContextImpl;
using SearchContext = std::shared_ptr<SearchContextImpl>;
using SearchContextList = std::vector<SearchContext>;

class GeneralizedSearchContextImpl;
using GeneralizedSearchContext = std::shared_ptr<GeneralizedSearchContextImpl>;

// StateRepositoryImpl
class StateRepositoryImpl;
using StateRepository = std::shared_ptr<StateRepositoryImpl>;

// PackedState
class PackedStateImpl;
using PackedState = const PackedStateImpl*;
template<typename T>
using PackedStateMap = UnorderedMap<PackedState, T>;

// UnpackedStateImpl
class UnpackedStateImpl;

// State
class State;

/* Grounder */
class Grounder;
class LiftedGrounder;

class ActionSatisficingBindingGenerator;
class AxiomSatisficingBindingGenerator;
class ConjunctiveConditionSatisficingBindingGenerator;

namespace satisficing_binding_generator
{
struct Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

/* ApplicableActionGenerators */
class IApplicableActionGenerator;
using ApplicableActionGenerator = std::shared_ptr<IApplicableActionGenerator>;
class GroundedApplicableActionGeneratorImpl;
using GroundedApplicableActionGenerator = std::shared_ptr<GroundedApplicableActionGeneratorImpl>;
class KPKCLiftedApplicableActionGeneratorImpl;
using KPKCLiftedApplicableActionGenerator = std::shared_ptr<KPKCLiftedApplicableActionGeneratorImpl>;
class ExhaustiveLiftedApplicableActionGeneratorImpl;
using ExhaustiveLiftedApplicableActionGenerator = std::shared_ptr<ExhaustiveLiftedApplicableActionGeneratorImpl>;

namespace applicable_action_generator::grounded
{
struct Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

namespace applicable_action_generator::lifted
{
namespace kpkc
{
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}
namespace exhaustive
{
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}
}

/* AxiomEvaluators */
class IAxiomEvaluator;
using AxiomEvaluator = std::shared_ptr<IAxiomEvaluator>;
class GroundedAxiomEvaluatorImpl;
using GroundedAxiomEvaluator = std::shared_ptr<GroundedAxiomEvaluatorImpl>;
class KPKCLiftedAxiomEvaluatorImpl;
using KPKCLiftedAxiomEvaluator = std::shared_ptr<KPKCLiftedAxiomEvaluatorImpl>;
class ExhaustiveLiftedAxiomEvaluatorImpl;
using ExhaustiveLiftedAxiomEvaluator = std::shared_ptr<ExhaustiveLiftedAxiomEvaluatorImpl>;

namespace axiom_evaluator::grounded
{
struct Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

namespace axiom_evaluator::lifted
{
namespace kpkc
{
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}
namespace exhaustive
{
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}
}

/* Heuristics */
class IHeuristic;
using Heuristic = std::shared_ptr<IHeuristic>;
class PerfectHeuristicImpl;
using PerfectHeuristic = std::shared_ptr<PerfectHeuristicImpl>;
class BlindHeuristicImpl;
using BlindHeuristic = std::shared_ptr<BlindHeuristicImpl>;
class MaxHeuristicImpl;
using MaxHeuristic = std::shared_ptr<MaxHeuristicImpl>;
class AddHeuristicImpl;
using AddHeuristic = std::shared_ptr<AddHeuristicImpl>;
class SetAddHeuristicImpl;
using SetAddHeuristic = std::shared_ptr<SetAddHeuristicImpl>;
class FFHeuristicImpl;
using FFHeuristic = std::shared_ptr<FFHeuristicImpl>;

/* Algorithms */
class IPruningStrategy;
using PruningStrategy = std::shared_ptr<IPruningStrategy>;
class NoPruningStrategyImpl;
using NoPruningStrategy = std::shared_ptr<NoPruningStrategyImpl>;
class DuplicatePruningStrategyImpl;
using DuplicatePruningStrategy = std::shared_ptr<DuplicatePruningStrategyImpl>;
namespace iw
{
class ArityZeroNoveltyPruningStrategyImpl;
using ArityZeroNoveltyPruningStrategy = std::shared_ptr<ArityZeroNoveltyPruningStrategyImpl>;
class ArityKNoveltyPruningStrategyImpl;
using ArityKNoveltyPruningStrategy = std::shared_ptr<ArityKNoveltyPruningStrategyImpl>;
}

class IGoalStrategy;
using GoalStrategy = std::shared_ptr<IGoalStrategy>;
class ProblemGoalStrategyImpl;
using ProblemGoalStrategy = std::shared_ptr<ProblemGoalStrategyImpl>;
class IExplorationStrategy;
using ExplorationStategy = std::shared_ptr<IExplorationStrategy>;

// AStar_EAGER
namespace astar_eager
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// AStar_LAZY
namespace astar_lazy
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// Breadth-first search
namespace brfs
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// GBFS_EAGER
namespace gbfs_eager
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// GBFS_LAZY
namespace gbfs_lazy
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// Iterative width search
namespace iw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;

/**
 * Large k for IW(k) is generally infeasible.
 * Hence, we use a small constant value within feasible range
 * allowing us to use stack allocated arrays.
 */

const size_t MAX_ARITY = 6;

/**
 * Number of initial atoms in the DynamicNoveltyTable
 */

const size_t INITIAL_TABLE_ATOMS = 64;

/**
 * Type aliases for readability
 */

using AtomIndex = Index;
using AtomIndexList = std::vector<AtomIndex>;

using TupleIndex = Index;
using TupleIndexList = std::vector<TupleIndex>;
using TupleIndexSet = std::unordered_set<TupleIndex>;
}

// Serialized iterative width search
namespace siw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
class Statistics;
}

// Plan
class Plan;

// PartiallyOrderedPlan
class PartiallyOrderedPlan;

namespace match_tree
{
template<mimir::formalism::HasConjunctiveCondition E>
class IInverseNode;

template<mimir::formalism::HasConjunctiveCondition E>
class INode;
}

}

#endif
