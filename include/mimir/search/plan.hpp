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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

class Plan
{
private:
    SearchContext m_context;
    StateList m_states;
    formalism::GroundActionList m_actions;
    ContinuousCost m_cost;

public:
    Plan(SearchContext context, StateList states, formalism::GroundActionList actions, ContinuousCost cost);

    const SearchContext& get_search_context() const;
    const StateList& get_states() const;
    const formalism::GroundActionList& get_actions() const;
    ContinuousCost get_cost() const;
    size_t get_length() const;
};

extern std::ostream& operator<<(std::ostream& os, const search::Plan& plan);

}

#endif
