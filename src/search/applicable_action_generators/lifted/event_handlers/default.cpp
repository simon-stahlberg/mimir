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

#include "mimir/search/applicable_action_generators/lifted/event_handlers/default.hpp"

using namespace mimir::formalism;

namespace mimir::search::applicable_action_generator::lifted
{
void DefaultEventHandlerImpl::on_start_generating_applicable_actions_impl() const {}

void DefaultEventHandlerImpl::on_ground_action_impl(GroundAction action) const {}

void DefaultEventHandlerImpl::on_ground_action_cache_hit_impl(GroundAction action) const {}

void DefaultEventHandlerImpl::on_ground_action_cache_miss_impl(GroundAction action) const {}

void DefaultEventHandlerImpl::on_end_generating_applicable_actions_impl() const {}

void DefaultEventHandlerImpl::on_finish_search_layer_impl() const {}

void DefaultEventHandlerImpl::on_end_search_impl() const { std::cout << get_statistics() << std::endl; }

DefaultEventHandlerImpl::DefaultEventHandlerImpl(bool quiet) : EventHandlerBase<DefaultEventHandlerImpl>(quiet) {}

std::shared_ptr<DefaultEventHandlerImpl> DefaultEventHandlerImpl::create(bool quiet) { return std::make_shared<DefaultEventHandlerImpl>(quiet); }
}
