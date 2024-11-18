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

#ifndef MIMIR_SEARCH_HEURISTICS_HSTAR_HPP_
#define MIMIR_SEARCH_HEURISTICS_HSTAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/heuristics/interface.hpp"

namespace mimir
{

/// @brief `HStarHeuristic` returns the shortest goal distance.
class HStarHeuristic : public IHeuristic
{
public:
    HStarHeuristic(Problem problem,
                   std::shared_ptr<PDDLRepositories> factories,
                   std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
                   std::shared_ptr<StateRepository> state_repository);

    double compute_heuristic(State state) override;

private:
    StateMap<double> m_estimates;
};

}

#endif
