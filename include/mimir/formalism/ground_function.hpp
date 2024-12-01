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

namespace mimir
{
class GroundFunctionImpl
{
private:
    Index m_index;
    FunctionSkeleton m_function_skeleton;
    ObjectList m_objects;

    GroundFunctionImpl(Index index, FunctionSkeleton function_skeleton, ObjectList objects);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundFunctionImpl(const GroundFunctionImpl& other) = delete;
    GroundFunctionImpl& operator=(const GroundFunctionImpl& other) = delete;
    GroundFunctionImpl(GroundFunctionImpl&& other) = default;
    GroundFunctionImpl& operator=(GroundFunctionImpl&& other) = default;

    Index get_index() const;
    const FunctionSkeleton& get_function_skeleton() const;
    const ObjectList& get_objects() const;
};

extern std::ostream& operator<<(std::ostream& out, const GroundFunctionImpl& element);

extern std::ostream& operator<<(std::ostream& out, GroundFunction element);

}

#endif
