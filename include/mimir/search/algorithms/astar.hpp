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

#ifndef MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_
#define MIMIR_SEARCH_ALGORITHMS_ASTAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/algorithms/utils.hpp"
#include "mimir/search/declarations.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace mimir
{

extern SearchResult find_solution_astar(std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                                        std::shared_ptr<StateRepository> state_repository,
                                        std::shared_ptr<IHeuristic> heuristic,
                                        std::optional<State> start_state = std::nullopt,
                                        std::optional<std::shared_ptr<IAStarAlgorithmEventHandler>> event_handler = std::nullopt,
                                        std::optional<std::shared_ptr<IGoalStrategy>> goal_strategy = std::nullopt,
                                        std::optional<std::shared_ptr<IPruningStrategy>> pruning_strategy = std::nullopt);

}

#endif
