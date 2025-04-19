/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_GRAPHS_COLOR_HPP_
#define MIMIR_GRAPHS_COLOR_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/graphs/declarations.hpp"

#include <cstddef>
#include <loki/loki.hpp>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>

namespace mimir::graphs
{
namespace nb = nanobind;

/// @brief `IColor` is the abstract base class for a color in vertex-colored graphs.
/// https://nanobind.readthedocs.io/en/latest/ownership_adv.html#intrusive-reference-counting
class IColor : public nb::intrusive_base
{
public:
    virtual ~IColor() = default;

    bool operator==(const IColor& other) const;
    bool operator<(const IColor& other) const;

    virtual bool equal_to(const IColor& other) const = 0;
    virtual bool less(const IColor& other) const = 0;
    virtual std::string str() const = 0;
    virtual size_t hash() const = 0;
};

template<typename... Ts>
class VariadicColor : public IColor
{
public:
    VariadicColor(Ts... colors) : m_colors(std::move(colors)...) { std::cout << "VARIADIC" << std::endl; }

    const std::tuple<Ts...>& get_colors() const { return m_colors; }

    bool equal_to(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const VariadicColor<Ts...>&>(other);
            return loki::EqualTo<std::tuple<Ts...>>()(m_colors, other_derived.m_colors);
        }
        return false;
    }

    bool less(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const VariadicColor<Ts...>&>(other);

            return m_colors < other_derived.m_colors;
        }
        return typeid(*this).before(typeid(other));
    }

    std::string str() const override
    {
        auto ss = std::stringstream {};
        mimir::operator<<(ss, m_colors);
        return ss.str();
    }

    size_t hash() const override { return loki::Hash<std::tuple<Ts...>>()(get_colors()); }

private:
    std::tuple<Ts...> m_colors;
};

class Color
{
public:
    /// @brief The constructor for Python managed `IColor`.
    /// Notice that Python users have to manually declare different types of colors by deriving from IColor
    /// while C++ users can make usage of the `VariadicColor` in the constructor below.
    /// @param color
    explicit Color(nb::ref<IColor> color);

    /// @brief The variadic constructor for C++ managed `IColor`.
    template<typename... Ts>
    Color(Ts... colors) : m_color(new VariadicColor<Ts...>(std::move(colors)...))
    {
    }

    bool operator==(const Color& other) const;
    bool operator<(const Color& other) const;
    std::string str() const;
    size_t hash() const;

private:
    nb::ref<IColor> m_color;
};

extern std::ostream& operator<<(std::ostream& out, const IColor& color);

extern std::ostream& operator<<(std::ostream& out, const Color& color);

}

template<>
struct loki::Hash<mimir::graphs::IColor>
{
    size_t operator()(const mimir::graphs::IColor& color) const;
};

template<>
struct loki::EqualTo<mimir::graphs::IColor>
{
    bool operator()(const mimir::graphs::IColor& lhs, const mimir::graphs::IColor& rhs) const;
};

template<>
struct loki::Hash<mimir::graphs::Color>
{
    size_t operator()(const mimir::graphs::Color& color) const;
};

#endif