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

#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/formalism.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/planners/interface.hpp"

#include <tuple>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class SinglePlanner : public IPlanner
{
private:
    std::shared_ptr<IAlgorithm> m_algorithm;

public:
    SinglePlanner(std::shared_ptr<IAlgorithm> algorithm) : m_algorithm(std::move(algorithm)) {}

    std::tuple<SearchStatus, Plan> find_solution() override
    {
        auto action_view_list = GroundActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }
};

}

#endif
