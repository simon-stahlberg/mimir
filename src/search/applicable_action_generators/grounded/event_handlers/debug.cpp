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

#include "mimir/search/applicable_action_generators/grounded/event_handlers/debug.hpp"

using namespace mimir::formalism;

namespace mimir::search::applicable_action_generator::grounded
{

void DebugEventHandlerImpl::on_start_ground_action_instantiation_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started instantiating ground actions." << std::endl;
}

void DebugEventHandlerImpl::on_finish_ground_action_instantiation_impl(std::chrono::milliseconds total_time) const
{
    std::cout << "[GroundedApplicableActionGenerator] Total time for ground action instantiation: " << total_time.count() << " ms" << std::endl;
}

void DebugEventHandlerImpl::on_start_build_action_match_tree_impl() const
{
    std::cout << "[GroundedApplicableActionGenerator] Started building action match tree." << std::endl;
}

void DebugEventHandlerImpl::on_finish_build_action_match_tree_impl(const match_tree::MatchTreeImpl<GroundActionImpl>& match_tree)
{
    std::cout << match_tree.get_statistics() << "\n"
              << "[GroundedApplicableActionGenerator] Finished building action match tree" << std::endl;
}

void DebugEventHandlerImpl::on_finish_search_layer_impl() const
{  //
}

void DebugEventHandlerImpl::on_end_search_impl() const {}

DebugEventHandlerImpl::DebugEventHandlerImpl(bool quiet) : EventHandlerBase<DebugEventHandlerImpl>(quiet) {}

std::shared_ptr<DebugEventHandlerImpl> DebugEventHandlerImpl::create(bool quiet) { return std::make_shared<DebugEventHandlerImpl>(quiet); }
}