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

namespace mimir
{

// StateRepository
class StateRepository;

// GroundAction
struct GroundActionImpl;
using GroundAction = const GroundActionImpl*;
using GroundActionList = std::vector<GroundAction>;
using GroundActionSet = std::unordered_set<GroundAction>;

// Axiom
struct GroundAxiomImpl;
using GroundAxiom = const GroundAxiomImpl*;
using GroundAxiomList = std::vector<GroundAxiom>;
using GroundAxiomSet = std::unordered_set<GroundAxiom>;

// State
struct StateImpl;
using State = const StateImpl*;
using StateList = std::vector<State>;
using StateSet = std::unordered_set<State>;
template<typename T>
using StateMap = std::unordered_map<State, T>;

/* ApplicableActionGenerators */
class IApplicableActionGenerator;
class GroundedApplicableActionGenerator;
class LiftedApplicableActionGenerator;

class ActionGrounder;

class IGroundedApplicableActionGeneratorEventHandler;
class ILiftedApplicableActionGeneratorEventHandler;

/* AxiomEvaluators */
class IAxiomEvaluator;
class GroundedAxiomEvaluator;
class LiftedAxiomEvaluator;

class AxiomGrounder;

// MatchTree
template<typename T>
class MatchTree;

/* Heuristics */
class IHeuristic;

/* Algorithms */
class IAlgorithm;

class IPruningStrategy;
class IGoalStrategy;

// AStar
class IAStarAlgorithmEventHandler;

// Breadth-first search
class IBrFSAlgorithmEventHandler;

// Iterative width search
class IIWAlgorithmEventHandler;

// Serialized iterative width search
class ISIWAlgorithmEventHandler;

// Plan
class Plan;

}

#endif
