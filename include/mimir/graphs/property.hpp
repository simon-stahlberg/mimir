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

#ifndef MIMIR_GRAPHS_PROPERTY_HPP_
#define MIMIR_GRAPHS_PROPERTY_HPP_

#include "mimir/common/formatter.hpp"
#include "mimir/graphs/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <loki/loki.hpp>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>

namespace mimir::graphs
{
/**
 * Polymorphic properties (for Python)
 */
class IProperty
{
public:
    virtual ~IProperty() = default;

    virtual bool operator==(const IProperty& other) const = 0;
    virtual bool operator<(const IProperty& other) const = 0;
    virtual std::string str() const = 0;
    virtual size_t hash() const = 0;
};

template<Property... Ps>
class Properties : public IProperty
{
public:
    Properties(Ps... properties) : m_properties(std::make_tuple(std::move(properties)...)) {}

    bool operator==(const Properties other) const { return loki::EqualTo<std::tuple<Ps...>> {}(m_properties, other.m_properties); }
    bool operator<(const Properties other) const { return m_properties < other.m_properties; }

    bool operator==(const IProperty& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const Properties&>(other);

            return *this == other_derived;
        }
        return false;
    }
    bool operator<(const IProperty& other) const override
    {
        if (typeid(*this) == typeid(other))
        {
            const auto& other_derived = static_cast<const Properties&>(other);

            return *this < other_derived;
        }
        return typeid(*this).before(typeid(other));
    }
    std::string str() const override
    {
        auto ss = std::stringstream {};
        ss << m_properties;
        return ss.str();
    }
    size_t hash() const override { return loki::Hash<std::tuple<Ps...>>()(m_properties); }

private:
    std::tuple<Ps...> m_properties;
};

class PropertyValue
{
public:
    explicit PropertyValue(std::shared_ptr<IProperty> property);

    template<Property... Ps>
    explicit PropertyValue(Ps... properties) : m_property(std::make_shared<Properties<Ps...>>(std::move(properties)...))
    {
    }

    bool operator==(const PropertyValue& other) const;
    bool operator<(const PropertyValue& other) const;
    std::string str() const;
    size_t hash() const;

private:
    std::shared_ptr<IProperty> m_property;
};

using PropertyValueList = std::vector<PropertyValue>;

}

namespace loki
{
template<>
struct Hash<mimir::graphs::IProperty>
{
    size_t operator()(const mimir::graphs::IProperty& property) const;
};

/// @private
template<>
struct EqualTo<mimir::graphs::IProperty>
{
    bool operator()(const mimir::graphs::IProperty& lhs, const mimir::graphs::IProperty& rhs) const;
};

/// @private
template<>
struct Hash<mimir::graphs::PropertyValue>
{
    size_t operator()(const mimir::graphs::PropertyValue& property) const;
};

/// @private
template<>
struct EqualTo<mimir::graphs::PropertyValue>
{
    bool operator()(const mimir::graphs::PropertyValue& lhs, const mimir::graphs::PropertyValue& rhs) const;
};
}

#endif