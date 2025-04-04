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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/applicable_action_generators/lifted/event_handlers/base.hpp"

namespace mimir::search::applicable_action_generator::lifted
{
class DebugEventHandlerImpl : public EventHandlerBase<DebugEventHandlerImpl>
{
private:
    /* Implement EventHandlerBase interface */
    friend class EventHandlerBase<DebugEventHandlerImpl>;

    void on_start_generating_applicable_actions_impl() const;

    void on_ground_action_impl(formalism::GroundAction action) const;

    void on_ground_action_cache_hit_impl(formalism::GroundAction action) const;

    void on_ground_action_cache_miss_impl(formalism::GroundAction action) const;

    void on_end_generating_applicable_actions_impl() const;

    void on_finish_search_layer_impl() const;

    void on_end_search_impl() const;

public:
    explicit DebugEventHandlerImpl(bool quiet = true);

    static DebugEventHandler create(bool quiet = true);
};
}

#endif
