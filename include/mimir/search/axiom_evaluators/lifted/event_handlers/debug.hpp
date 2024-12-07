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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/axiom_evaluators/lifted/event_handlers/interface.hpp"

namespace mimir
{

class DebugLiftedAxiomEvaluatorEventHandler : public LiftedAxiomEvaluatorEventHandlerBase<DebugLiftedAxiomEvaluatorEventHandler>
{
private:
    /* Implement LiftedAxiomEvaluatorEventHandlerBase interface */
    friend class LiftedAxiomEvaluatorEventHandlerBase<DebugLiftedAxiomEvaluatorEventHandler>;

    void on_start_generating_applicable_axioms_impl() const;

    void on_ground_axiom_impl(GroundAxiom axiom) const;

    void on_ground_axiom_cache_hit_impl(GroundAxiom axiom) const;

    void on_ground_axiom_cache_miss_impl(GroundAxiom axiom) const;

    void on_end_generating_applicable_axioms_impl() const;

    void on_finish_search_layer_impl() const;

    void on_end_search_impl() const;

public:
    explicit DebugLiftedAxiomEvaluatorEventHandler(bool quiet = true) : LiftedAxiomEvaluatorEventHandlerBase<DebugLiftedAxiomEvaluatorEventHandler>(quiet) {}
};

}

#endif
