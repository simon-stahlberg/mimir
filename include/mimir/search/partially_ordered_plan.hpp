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

#ifndef MIMIR_SEARCH_PARTIALLY_ORDERED_PLAN_HPP_
#define MIMIR_SEARCH_PARTIALLY_ORDERED_PLAN_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/plan.hpp"

namespace mimir::search
{

class PartiallyOrderedPlan
{
private:
    Plan m_t_o_plan;
    std::vector<std::vector<bool>> m_precedes;

public:
    explicit PartiallyOrderedPlan(Plan t_o_plan);

    bool must_precede(size_t i, size_t j) const;
    Plan compute_t_o_plan_with_minimal_context_switches() const;

    const Plan& get_t_o_plan() const;
    const std::vector<std::vector<bool>>& get_precedes() const;
};
}

#endif
