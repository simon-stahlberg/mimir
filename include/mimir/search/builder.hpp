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

#ifndef MIMIR_SEARCH_BUILDER_HPP_
#define MIMIR_SEARCH_BUILDER_HPP_

#include <flatmemory/flatmemory.hpp>

namespace mimir
{
/**
 * Interface class
 */
template<typename Derived>
class IBuilder
{
private:
    IBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] auto& get_flatmemory_builder() { return self().get_flatmemory_builder_impl(); }
    [[nodiscard]] const auto& get_flatmemory_builder() const { return self().get_flatmemory_builder_impl(); }
};

/**
 * Implementation class
 */
template<typename Tag>
class Builder : public IBuilder<Builder<Tag>>
{
};
}

#endif
