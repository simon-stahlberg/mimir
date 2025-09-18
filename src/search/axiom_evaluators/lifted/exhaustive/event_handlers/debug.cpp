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

#include "mimir/search/axiom_evaluators/lifted/exhaustive/event_handlers/debug.hpp"

using namespace mimir::formalism;

namespace mimir::search::axiom_evaluator::lifted::exhaustive
{
void DebugEventHandlerImpl::on_start_generating_applicable_axioms_impl() const {}

void DebugEventHandlerImpl::on_ground_axiom_impl(GroundAxiom axiom) const {}

void DebugEventHandlerImpl::on_ground_axiom_cache_hit_impl(GroundAxiom axiom) const {}

void DebugEventHandlerImpl::on_ground_axiom_cache_miss_impl(GroundAxiom axiom) const {}

void DebugEventHandlerImpl::on_end_generating_applicable_axioms_impl() const {}

void DebugEventHandlerImpl::on_finish_search_layer_impl() const {}

void DebugEventHandlerImpl::on_end_search_impl() const { std::cout << get_statistics() << std::endl; }

DebugEventHandlerImpl::DebugEventHandlerImpl(bool quiet) : EventHandlerBase<DebugEventHandlerImpl>(quiet) {}

std::shared_ptr<DebugEventHandlerImpl> DebugEventHandlerImpl::create(bool quiet) { return std::make_shared<DebugEventHandlerImpl>(quiet); }
}