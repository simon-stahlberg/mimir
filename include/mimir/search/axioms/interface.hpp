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

#ifndef MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_

#include "mimir/search/axioms/tags.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived_>
class IAxiomBuilder
{
private:
    IAxiomBuilder() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    [[nodiscard]] uint32_t& get_id() { return self().get_id_impl(); }
    [[nodiscard]] Axiom& get_axiom() { return self().get_axiom_impl(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects() { return self().get_objects_impl(); }
};

/**
 * Interface class
 */
template<typename Derived_>
class IAxiomView
{
private:
    IAxiomView() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    [[nodiscard]] uint32_t get_id() const { return self().get_id_impl(); }
    [[nodiscard]] Axiom get_axiom() const { return self().get_axiom_impl(); }
    [[nodiscard]] FlatObjectList get_objects() const { return self().get_objects_impl(); }
};

}

#endif