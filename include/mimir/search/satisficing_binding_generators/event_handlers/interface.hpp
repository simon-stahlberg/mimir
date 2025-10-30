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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATORS_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATORS_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/event_handlers/statistics.hpp"

namespace mimir::search::satisficing_binding_generator
{
class IEventHandler
{
public:
    virtual ~IEventHandler() = default;

    virtual void on_valid_base_binding(const formalism::ObjectList& binding) = 0;
    virtual void on_valid_derived_binding(const formalism::ObjectList& binding) = 0;

    virtual void on_invalid_base_binding(const formalism::ObjectList& binding) = 0;
    virtual void on_invalid_derived_binding(const formalism::ObjectList& binding) = 0;

    virtual void on_end_search() = 0;

    virtual void on_finish_search_layer() = 0;

    virtual const Statistics& get_statistics() const = 0;
};

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

    void on_valid_base_binding(const formalism::ObjectList& binding) override
    {
        m_statistics.increment_num_valid_base_bindings();

        if (!m_quiet)
        {
            self().on_valid_base_binding_impl(binding);
        }
    }

    void on_valid_derived_binding(const formalism::ObjectList& binding) override
    {
        m_statistics.increment_num_valid_derived_bindings();

        if (!m_quiet)
        {
            self().on_valid_derived_binding_impl(binding);
        }
    }

    void on_invalid_base_binding(const formalism::ObjectList& binding) override
    {
        m_statistics.increment_num_invalid_base_bindings();

        if (!m_quiet)
        {
            self().on_invalid_base_binding_impl(binding);
        }
    }

    void on_invalid_derived_binding(const formalism::ObjectList& binding) override
    {
        m_statistics.increment_num_invalid_derived_bindings();

        if (!m_quiet)
        {
            self().on_invalid_derived_binding_impl(binding);
        }
    }

    void on_end_search() override
    {
        if (!m_quiet)
        {
            self().on_end_search_impl();
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

    const Statistics& get_statistics() const override { return m_statistics; }
};

}

#endif