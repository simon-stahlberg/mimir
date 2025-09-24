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

#include "mimir/search/satisficing_binding_generators/event_handlers/default.hpp"

namespace mimir::search::satisficing_binding_generator
{
void DefaultEventHandlerImpl::on_valid_base_binding_impl(const formalism::ObjectList& binding) const {}
void DefaultEventHandlerImpl::on_valid_derived_binding_impl(const formalism::ObjectList& binding) const {}

void DefaultEventHandlerImpl::on_invalid_base_binding_impl(const formalism::ObjectList& binding) const {}
void DefaultEventHandlerImpl::on_invalid_derived_binding_impl(const formalism::ObjectList& binding) const {}

void DefaultEventHandlerImpl::on_end_search_impl() const { std::cout << get_statistics() << std::endl; }

void DefaultEventHandlerImpl::on_finish_search_layer_impl() const {}

DefaultEventHandlerImpl::DefaultEventHandlerImpl(bool quiet) : EventHandlerBase<DefaultEventHandlerImpl>(quiet) {}

DefaultEventHandler DefaultEventHandlerImpl::create(bool quiet) { return std::make_shared<DefaultEventHandlerImpl>(quiet); }

}