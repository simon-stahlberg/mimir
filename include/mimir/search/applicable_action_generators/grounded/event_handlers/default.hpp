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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_DEFAULT_HPP_

#include "mimir/search/applicable_action_generators/grounded/event_handlers/base.hpp"

namespace mimir::search::applicable_action_generator::grounded
{
class DefaultEventHandlerImpl : public EventHandlerBase<DefaultEventHandlerImpl>
{
private:
    /* Implement EventHandlerBase interface */
    friend class EventHandlerBase<DefaultEventHandlerImpl>;

    void on_start_ground_action_instantiation_impl() const;

    void on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const;

    void on_start_build_action_match_tree_impl() const;

    void on_finish_build_action_match_tree_impl(const match_tree::MatchTreeImpl<formalism::GroundActionImpl>& action_match_tree);

    void on_finish_search_layer_impl() const;

    void on_end_search_impl() const;

public:
    explicit DefaultEventHandlerImpl(bool quiet = true);

    static std::shared_ptr<DefaultEventHandlerImpl> create(bool quiet = true);
};
}

#endif
