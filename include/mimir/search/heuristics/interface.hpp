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

#ifndef MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_
#define MIMIR_SEARCH_HEURISTICS_INTERFACE_HPP_

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/heuristics/tags.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Interface class
 */
class IDynamicHeuristic
{
public:
    virtual ~IDynamicHeuristic() = default;

    [[nodiscard]] virtual double compute_heuristic(State state) = 0;
};

/**
 * Interface class
 */
template<typename Derived_>
class IStaticHeuristic : public IDynamicHeuristic
{
private:
    IStaticHeuristic() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    [[nodiscard]] double compute_heuristic(State state) override { return self().compute_heuristic_impl(state); }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsHeuristicDispatcher T>
class Heuristic : public IStaticHeuristic<Heuristic<T>>
{
};

}

#endif