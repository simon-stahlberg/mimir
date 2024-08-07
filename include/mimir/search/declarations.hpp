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

#include <memory>
#include <unordered_set>
#include <vector>

namespace mimir
{

/* Algorithms */
class IApplicableActionGenerator;

class IAlgorithm;

class IPruningStrategy;
class IGoalStrategy;

class IBrFSAlgorithmEventHandler;
using BrFSAlgorithmEventHandler = std::shared_ptr<IBrFSAlgorithmEventHandler>;
class IIWAlgorithmEventHandler;
using IWAlgorithmEventHandler = std::shared_ptr<IIWAlgorithmEventHandler>;
class ISIWAlgorithmEventHandler;
using SIWAlgorithmEventHandler = std::shared_ptr<ISIWAlgorithmEventHandler>;

/* Heuristics */
class IHeuristic;

class GroundAction;
using GroundActionIndex = uint32_t;
using GroundActionCost = double;

class GroundAxiom;
using GroundAxiomIndex = uint32_t;

class State;
using StateIndex = uint32_t;
using StateIndexList = std::vector<StateIndex>;
using StateIndexSet = std::unordered_set<StateIndex>;

class StateRepository;

}

#endif
