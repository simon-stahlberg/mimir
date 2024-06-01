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

#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{
/**
 * Dynamic interface class
 */
class IDynamicSSG
{
public:
    virtual ~IDynamicSSG() = default;

    [[nodiscard]] virtual State get_or_create_initial_state() = 0;

    /// @brief Expert interface for creating states.
    /// The user must ensure that the atoms are part of the problem from the applicable action generator.
    [[nodiscard]] virtual State get_or_create_state(const GroundAtomList<Fluent>& atoms) = 0;

    [[nodiscard]] virtual State get_or_create_successor_state(const State state, const GroundAction action) = 0;

    [[nodiscard]] virtual size_t get_state_count() const = 0;
};

/**
 * Static interface class
 */
template<typename Derived_>
class IStaticSSG : public IDynamicSSG
{
private:
    IStaticSSG() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    [[nodiscard]] State get_or_create_initial_state() override
    {  //
        return self().get_or_create_initial_state_impl();
    }

    [[nodiscard]] State get_or_create_state(const GroundAtomList<Fluent>& atoms) override
    {  //
        return self().get_or_create_state_impl(atoms);
    }

    [[nodiscard]] State get_or_create_successor_state(const State state, const GroundAction action) override
    {  //
        return self().get_or_create_successor_state_impl(state, action);
    }

    [[nodiscard]] size_t get_state_count() const override
    {  //
        return self().get_state_count_impl();
    }
};

/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
 */
template<IsStateTag S>
class SSG : public IStaticSSG<SSG<S>>
{
};

}

#endif
