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

#include <memory>
#include <unordered_set>
#include <vector>

namespace mimir::search
{

class SearchContext;

class GeneralizedSearchContext;

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

/* ApplicableActionGenerators */
class IApplicableActionGenerator;
using ApplicableActionGenerator = std::shared_ptr<IApplicableActionGenerator>;
class GroundedApplicableActionGenerator;
class LiftedApplicableActionGenerator;

/* AxiomEvaluators */
class IAxiomEvaluator;
using AxiomEvaluator = std::shared_ptr<IAxiomEvaluator>;
class GroundedAxiomEvaluator;
class LiftedAxiomEvaluator;

/* MatchTree */
template<typename T>
class MatchTree;

/* Heuristics */
class IHeuristic;
using Heuristic = std::shared_ptr<IHeuristic>;

/* Algorithms */

class IPruningStrategy;
using PruningStrategy = std::shared_ptr<IPruningStrategy>;
class IGoalStrategy;
using GoalStrategy = std::shared_ptr<IGoalStrategy>;

// AStar
namespace astar
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
}

// Breadth-first search
namespace brfs
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
}

// Iterative width search
namespace iw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
}

// Serialized iterative width search
namespace siw
{
class IEventHandler;
using EventHandler = std::shared_ptr<IEventHandler>;
}

// Plan
class Plan;

}

#endif
