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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_KPKC_EVENT_HANDLERS_BASE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_KPKC_EVENT_HANDLERS_BASE_HPP_

#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc/event_handlers/statistics.hpp"

namespace mimir::search::applicable_action_generator::lifted::kpkc
{

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived>
class EventHandlerBase : public IEventHandler
{
protected:
    Statistics m_statistics;
    bool m_quiet;

private:
    EventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    explicit EventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_generating_applicable_actions() override
    {
        if (!m_quiet)
            self().on_start_generating_applicable_actions_impl();
    }

    void on_ground_action(formalism::GroundAction action) override
    {
        if (!m_quiet)
            self().on_ground_action_impl(action);
    }

    void on_ground_action_cache_hit(formalism::GroundAction action) override
    {
        m_statistics.increment_num_ground_action_cache_hits();

        if (!m_quiet)
            self().on_ground_action_cache_hit_impl(action);
    }

    void on_ground_action_cache_miss(formalism::GroundAction action) override
    {
        m_statistics.increment_num_ground_action_cache_misses();

        if (!m_quiet)
            self().on_ground_action_cache_miss_impl(action);
    }

    void on_end_generating_applicable_actions() override
    {
        if (!m_quiet)
            self().on_end_generating_applicable_actions_impl();
    }

    void on_finish_search_layer() override
    {
        m_statistics.on_finish_search_layer();

        if (!m_quiet)
            self().on_finish_search_layer_impl();
    }

    void on_end_search() override
    {
        if (!m_quiet)
            self().on_end_search_impl();
    }

    const Statistics& get_statistics() const override { return m_statistics; }
};
}

#endif