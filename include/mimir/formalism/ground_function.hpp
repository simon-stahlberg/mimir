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

#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"

#include <loki/loki.hpp>
#include <string>
#include <vector>

namespace mimir
{
class GroundFunctionImpl : public loki::Base<GroundFunctionImpl>
{
private:
    FunctionSkeleton m_function_skeleton;
    ObjectList m_objects;

    GroundFunctionImpl(int identifier, FunctionSkeleton function_skeleton, ObjectList objects);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundFunctionImpl, loki::Hash<GroundFunctionImpl*>, loki::EqualTo<GroundFunctionImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundFunctionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundFunctionImpl>;

public:
    const FunctionSkeleton& get_function_skeleton() const;
    const ObjectList& get_objects() const;
};

/**
 * Type aliases
 */

using GroundFunction = const GroundFunctionImpl*;
using GroundFunctionList = std::vector<GroundFunction>;

}

#endif
