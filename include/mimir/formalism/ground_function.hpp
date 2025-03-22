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

#ifndef MIMIR_FORMALISM_GROUND_GROUND_FUNCTION_HPP_
#define MIMIR_FORMALISM_GROUND_GROUND_FUNCTION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
template<IsStaticOrFluentOrAuxiliaryTag F>
class GroundFunctionImpl
{
private:
    Index m_index;
    FunctionSkeleton<F> m_function_skeleton;
    ObjectList m_objects;

    GroundFunctionImpl(Index index, FunctionSkeleton<F> function_skeleton, ObjectList objects);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;
    using Type = F;

    // moveable but not copyable
    GroundFunctionImpl(const GroundFunctionImpl& other) = delete;
    GroundFunctionImpl& operator=(const GroundFunctionImpl& other) = delete;
    GroundFunctionImpl(GroundFunctionImpl&& other) = default;
    GroundFunctionImpl& operator=(GroundFunctionImpl&& other) = default;

    Index get_index() const;
    FunctionSkeleton<F> get_function_skeleton() const;
    const ObjectList& get_objects() const;
    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_function_skeleton(), std::cref(get_objects())); }
};

template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl<F>& element);

template<IsStaticOrFluentOrAuxiliaryTag F>
extern std::ostream& operator<<(std::ostream& out, GroundFunction<F> element);

}

#endif
