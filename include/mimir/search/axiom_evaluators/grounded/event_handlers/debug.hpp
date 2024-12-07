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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_GROUNDED_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_GROUNDED_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/axiom_evaluators/grounded/event_handlers/interface.hpp"

namespace mimir
{

class DebugGroundedAxiomEvaluatorEventHandler : public GroundedAxiomEvaluatorEventHandlerBase<DebugGroundedAxiomEvaluatorEventHandler>
{
private:
    /* Implement GroundedAxiomEvaluatorEventHandlerBase interface */
    friend class GroundedAxiomEvaluatorEventHandlerBase<DebugGroundedAxiomEvaluatorEventHandler>;

    void on_finish_delete_free_exploration_impl(const GroundAtomList<Fluent>& reached_fluent_atoms,
                                                const GroundAtomList<Derived>& reached_derived_atoms,
                                                const GroundAxiomList& instantiated_axioms);

    void on_finish_grounding_unrelaxed_axioms_impl(const GroundAxiomList& unrelaxed_axioms);

    void on_finish_build_axiom_match_tree_impl(const MatchTree<GroundAxiom>& axiom_match_tree);

    void on_finish_search_layer_impl() const;

    void on_end_search_impl() const;

public:
    explicit DebugGroundedAxiomEvaluatorEventHandler(bool quiet = true) : GroundedAxiomEvaluatorEventHandlerBase<DebugGroundedAxiomEvaluatorEventHandler>(quiet)
    {
    }
};

}

#endif