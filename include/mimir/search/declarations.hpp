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
#include "mimir/common/types.hpp"
#include "mimir/formalism/concepts.hpp"

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

// State
struct StateImpl;
using State = const StateImpl*;
using StateList = std::vector<State>;
using StateSet = std::unordered_set<State>;
template<typename T>
using StateMap = std::unordered_map<State, T>;

struct DenseState;

/* DeleteRelaxedProblemExplorator */
class DeleteRelaxedProblemExplorator;

class ActionSatisficingBindingGenerator;
class AxiomSatisficingBindingGenerator;
class ConjunctiveConditionSatisficingBindingGenerator;

namespace satisficing_binding_generator
{
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
class LiftedApplicableActionGeneratorImpl;
using LiftedApplicableActionGenerator = std::shared_ptr<LiftedApplicableActionGeneratorImpl>;

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
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

/* AxiomEvaluators */
class IAxiomEvaluator;
using AxiomEvaluator = std::shared_ptr<IAxiomEvaluator>;
class GroundedAxiomEvaluatorImpl;
using GroundedAxiomEvaluator = std::shared_ptr<GroundedAxiomEvaluatorImpl>;
class LiftedAxiomEvaluatorImpl;
using LiftedAxiomEvaluator = std::shared_ptr<LiftedAxiomEvaluatorImpl>;

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
class Statistics;
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

/* Heuristics */
class IHeuristic;
using Heuristic = std::shared_ptr<IHeuristic>;
class HStarHeuristicImpl;
using HStarHeuristic = std::shared_ptr<HStarHeuristicImpl>;
class BlindHeuristicImpl;
using BlindHeuristic = std::shared_ptr<BlindHeuristicImpl>;

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

// AStar
namespace astar
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DebugEventHandlerImpl;
using DebugEventHandler = std::shared_ptr<DebugEventHandlerImpl>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
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
}

// Iterative width search
namespace iw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

// Serialized iterative width search
namespace siw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
class DefaultEventHandlerImpl;
using DefaultEventHandler = std::shared_ptr<DefaultEventHandlerImpl>;
}

// Plan
class Plan;

}

#endif
