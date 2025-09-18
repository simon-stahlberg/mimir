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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EXHAUSTIVE_EVENT_HANDLERS_DEFAULT_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_LIFTED_EXHAUSTIVE_EVENT_HANDLERS_DEFAULT_HPP_

#include "mimir/search/axiom_evaluators/lifted/exhaustive/event_handlers/base.hpp"

namespace mimir::search::axiom_evaluator::lifted::exhaustive
{
class DefaultEventHandlerImpl : public EventHandlerBase<DefaultEventHandlerImpl>
{
private:
    /* Implement EventHandlerBase interface */
    friend class EventHandlerBase<DefaultEventHandlerImpl>;

    void on_start_generating_applicable_axioms_impl() const;

    void on_ground_axiom_impl(formalism::GroundAxiom axiom) const;

    void on_ground_axiom_cache_hit_impl(formalism::GroundAxiom axiom) const;

    void on_ground_axiom_cache_miss_impl(formalism::GroundAxiom axiom) const;

    void on_end_generating_applicable_axioms_impl() const;

    void on_finish_search_layer_impl() const;

    void on_end_search_impl() const;

public:
    explicit DefaultEventHandlerImpl(bool quiet = true);

    static std::shared_ptr<DefaultEventHandlerImpl> create(bool quiet = true);
};
}

#endif
