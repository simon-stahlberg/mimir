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

namespace mimir::graphs
{
class IColor
{
public:
    virtual ~IColor() = default;

    virtual bool operator==(const IColor& other) const = 0;
    virtual bool operator<(const IColor& other) const = 0;
    virtual std::ostream& operator<<(std::ostream& out) const = 0;

    virtual size_t hash() const = 0;
};

template<typename... Ts>
class VariadicColor : public IColor
{
public:
    VariadicColor(Ts... colors) : m_colors(std::move(colors)...) {}

    const std::tuple<Ts...>& get_colors() const { return m_colors; }

    bool operator==(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const VariadicColor<Ts...>&>(other);
            return loki::EqualTo<std::tuple<Ts...>>()(m_colors, other_derived.m_colors);
        }
        return false;
    }

    bool operator<(const IColor& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const VariadicColor<Ts...>&>(other);

            return m_colors < other_derived.m_colors;
        }
        return typeid(*this).before(typeid(other));
    }

    std::ostream& operator<<(std::ostream& out) const override
    {
        mimir::operator<<(out, m_colors);

        return out;
    }

    size_t hash() const override { return loki::Hash<std::tuple<Ts...>>()(get_colors()); }

private:
    std::tuple<Ts...> m_colors;
};

class Color
{
public:
    template<typename... Ts>
    Color(Ts... colors) : m_color(std::make_shared<VariadicColor<Ts...>>(std::move(colors)...))
    {
    }

    bool operator==(const Color& other) const;
    bool operator<(const Color& other) const;

    friend std::ostream& operator<<(std::ostream& out, const Color& color);
    size_t hash() const;

private:
    std::shared_ptr<IColor> m_color;
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