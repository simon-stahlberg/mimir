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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_KPKC_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_KPKC_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search::applicable_action_generator::lifted::kpkc
{
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;

    virtual void on_start_generating_applicable_actions() = 0;

    virtual void on_ground_action(formalism::GroundAction action) = 0;

    virtual void on_ground_action_cache_hit(formalism::GroundAction action) = 0;

    virtual void on_ground_action_cache_miss(formalism::GroundAction action) = 0;

    virtual void on_end_generating_applicable_actions() = 0;

    virtual void on_end_search() = 0;

    virtual void on_finish_search_layer() = 0;

    virtual const Statistics& get_statistics() const = 0;
};
}

#endif
