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

#ifndef MIMIR_FORMALISM_GROUND_FUNCTION_VALUE_HPP_
#define MIMIR_FORMALISM_GROUND_FUNCTION_VALUE_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionValueImpl
{
private:
    Index m_index;
    GroundFunction<F> m_function;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    GroundFunctionValueImpl(Index index, GroundFunction<F> function, double number);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;
    using Type = F;

    // moveable but not copyable
    GroundFunctionValueImpl(const GroundFunctionValueImpl& other) = delete;
    GroundFunctionValueImpl& operator=(const GroundFunctionValueImpl& other) = delete;
    GroundFunctionValueImpl(GroundFunctionValueImpl&& other) = default;
    GroundFunctionValueImpl& operator=(GroundFunctionValueImpl&& other) = default;

    Index get_index() const;
    GroundFunction<F> get_function() const;
    double get_number() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_function(), get_number()); }
};

template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionValueImpl<F>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, GroundFunctionValue<F> element);

}

#endif