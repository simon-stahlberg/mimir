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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_BASE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_EVENT_HANDLERS_BASE_HPP_

#include "mimir/search/applicable_action_generators/grounded/event_handlers/interface.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/statistics.hpp"

namespace mimir::search::applicable_action_generator::grounded
{
/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived_>
class EventHandlerBase : public IEventHandler
{
protected:
    Statistics m_statistics;
    bool m_quiet;

private:
    EventHandlerBase() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    explicit EventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_ground_action_instantiation() override
    {
        if (!m_quiet)
            self().on_start_ground_action_instantiation_impl();
    }

    void on_finish_ground_action_instantiation(std::chrono::milliseconds total_time) override
    {
        if (!m_quiet)
            self().on_finish_ground_action_instantiation_impl(total_time);
    }

    void on_start_build_action_match_tree() override
    {
        if (!m_quiet)
            self().on_start_build_action_match_tree_impl();
    }

    void on_finish_build_action_match_tree(const match_tree::MatchTreeImpl<formalism::GroundActionImpl>& match_tree) override
    {
        m_statistics.statistics = match_tree.get_statistics();

        if (!m_quiet)
            self().on_finish_build_action_match_tree_impl(match_tree);
    }

    void on_finish_search_layer() override
    {
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