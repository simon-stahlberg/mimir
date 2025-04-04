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

#include "mimir/search/axiom_evaluators/grounded/event_handlers/debug.hpp"

using namespace mimir::formalism;

namespace mimir::search::axiom_evaluator::grounded
{
void DebugEventHandlerImpl::on_start_ground_axiom_instantiation_impl() const
{
    std::cout << "[GroundedAxiomEvaluator] Started instantiating ground axioms." << std::endl;
}

void DebugEventHandlerImpl::on_finish_ground_axiom_instantiation_impl(std::chrono::milliseconds total_time) const
{
    std::cout << "[GroundedAxiomEvaluator] Total time for ground axiom instantiation: " << total_time.count() << " ms" << std::endl;
}

void DebugEventHandlerImpl::on_start_build_axiom_match_trees_impl() const
{
    std::cout << "[GroundedAxiomEvaluator] Started building axiom match trees." << std::endl;
}

void DebugEventHandlerImpl::on_start_build_axiom_match_tree_impl(size_t partition_index) const
{
    std::cout << "[GroundedAxiomEvaluator] Axiom partition: " << partition_index << std::endl;
}

void DebugEventHandlerImpl::on_finish_build_axiom_match_tree_impl(const match_tree::MatchTreeImpl<GroundAxiomImpl>& match_tree) const
{
    std::cout << match_tree.get_statistics() << std::endl;
}

void DebugEventHandlerImpl::on_finish_build_axiom_match_trees_impl(std::chrono::milliseconds total_time) const
{
    std::cout << "[GroundedAxiomEvaluator] Total time for building axiom match trees: " << total_time.count() << " ms" << std::endl;
}

void DebugEventHandlerImpl::on_finish_search_layer_impl() const {}

void DebugEventHandlerImpl::on_end_search_impl() const {}

DebugEventHandlerImpl::DebugEventHandlerImpl(bool quiet) : EventHandlerBase<DebugEventHandlerImpl>(quiet) {}

std::shared_ptr<DebugEventHandlerImpl> DebugEventHandlerImpl::create(bool quiet) { return std::make_shared<DebugEventHandlerImpl>(quiet); }
}