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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/lifted/event_handlers/statistics.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{
template<typename T>
class MatchTree;

/**
 * Interface class
 */
class ILiftedApplicableActionGeneratorEventHandler
{
public:
    virtual ~ILiftedApplicableActionGeneratorEventHandler() = default;

    virtual void on_start_generating_applicable_actions() = 0;

    virtual void on_ground_action(GroundAction action) = 0;

    virtual void on_ground_action_cache_hit(GroundAction action) = 0;

    virtual void on_ground_action_cache_miss(GroundAction action) = 0;

    virtual void on_end_generating_applicable_actions() = 0;

    virtual void on_end_search() = 0;

    virtual void on_finish_search_layer() = 0;

    virtual const LiftedApplicableActionGeneratorStatistics& get_statistics() const = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived_>
class LiftedApplicableActionGeneratorEventHandlerBase : public ILiftedApplicableActionGeneratorEventHandler
{
protected:
    LiftedApplicableActionGeneratorStatistics m_statistics;
    bool m_quiet;

private:
    LiftedApplicableActionGeneratorEventHandlerBase() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    explicit LiftedApplicableActionGeneratorEventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

    void on_start_generating_applicable_actions() override
    {
        if (!m_quiet)
        {
            self().on_start_generating_applicable_actions_impl();
        }
    }

    void on_ground_action(GroundAction action) override
    {
        if (!m_quiet)
        {
            self().on_ground_action_impl(action);
        }
    }

    void on_ground_action_cache_hit(GroundAction action) override
    {
        m_statistics.increment_num_ground_action_cache_hits();

        if (!m_quiet)
        {
            self().on_ground_action_cache_hit_impl(action);
        }
    }

    void on_ground_action_cache_miss(GroundAction action) override
    {
        m_statistics.increment_num_ground_action_cache_misses();

        if (!m_quiet)
        {
            self().on_ground_action_cache_miss_impl(action);
        }
    }

    void on_end_generating_applicable_actions() override
    {
        if (!m_quiet)
        {
            self().on_end_generating_applicable_actions_impl();
        }
    }

    void on_finish_search_layer() override
    {
        m_statistics.on_finish_search_layer();

        if (!m_quiet)
        {
            self().on_finish_search_layer_impl();
        }
    }

    void on_end_search() override
    {
        if (!m_quiet)
        {
            self().on_end_search_impl();
        }
    }

    const LiftedApplicableActionGeneratorStatistics& get_statistics() const override { return m_statistics; }
};
}

#endif
