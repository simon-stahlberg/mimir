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

#ifndef MIMIR_GRAPHS_DECLARATIONS_HPP_
#define MIMIR_GRAPHS_DECLARATIONS_HPP_

#include <concepts>
#include <cstddef>
#include <loki/loki.hpp>
#include <tuple>
#include <vector>

namespace mimir::graphs
{
template<class T>
concept Property = requires(const T& a, const T& b, std::ostream& os) {
    { loki::Hash<T> {}(a) } -> std::convertible_to<std::size_t>;
    { loki::EqualTo<T> {}(a, b) } -> std::same_as<bool>;
    { a < b } -> std::same_as<bool>;  // TODO: only necessary in wl, fwl currently
    { os << a } -> std::same_as<std::ostream&>;
};

/**
 * Templated properties
 */

template<Property... Ts>
using Properties = std::tuple<Ts...>;
template<Property... Ts>
using PropertiesList = std::vector<Properties<Ts...>>;

/**
 * Polymorphic properties (for Python)
 */
class IProperties
{
public:
    virtual ~IProperties() = default;

    virtual bool operator==(const IProperties& other) const = 0;
    virtual bool operator<(const IProperties& other) const = 0;
    virtual std::string str() const = 0;
    virtual size_t hash() const = 0;
};

class PropertyObserver
{
public:
    explicit PropertyObserver(std::shared_ptr<IProperties> color);

    bool operator==(const PropertyObserver& other) const;
    bool operator<(const PropertyObserver& other) const;
    std::string str() const;
    size_t hash() const;

private:
    std::shared_ptr<IProperties> m_property;
};

}

namespace loki
{
template<>
struct loki::Hash<mimir::graphs::IProperties>
{
    size_t operator()(const mimir::graphs::IProperties& property) const;
};

/// @private
template<>
struct loki::EqualTo<mimir::graphs::IProperties>
{
    bool operator()(const mimir::graphs::IProperties& lhs, const mimir::graphs::IProperties& rhs) const;
};

/// @private
template<>
struct loki::Hash<mimir::graphs::PropertyObserver>
{
    size_t operator()(const mimir::graphs::PropertyObserver& property) const;
};

/// @private
template<>
struct loki::EqualTo<mimir::graphs::PropertyObserver>
{
    size_t operator()(const mimir::graphs::PropertyObserver& lhs, const mimir::graphs::PropertyObserver& rhs) const;
};
}

namespace mimir::graphs
{
static_assert(Property<PropertyObserver>);
}

#endif