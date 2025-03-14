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

#ifndef MIMIR_SEARCH_PLAN_HPP_
#define MIMIR_SEARCH_PLAN_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

class Plan
{
private:
    formalism::GroundActionList m_actions;
    ContinuousCost m_cost;

public:
    Plan(formalism::GroundActionList actions, ContinuousCost cost);

    const formalism::GroundActionList& get_actions() const;

    ContinuousCost get_cost() const;
};

/// @brief Write the plan to an ostream.
extern std::ostream& operator<<(std::ostream& os, const std::tuple<const Plan&, const formalism::ProblemImpl&>& data);

}

#endif
