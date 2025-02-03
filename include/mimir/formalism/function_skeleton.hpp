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

#ifndef MIMIR_FORMALISM_FUNCTION_SKELETON_HPP_
#define MIMIR_FORMALISM_FUNCTION_SKELETON_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
enum class FunctionMonotonicityEnum
{
    MONOTONICALLY_INCREASING = 0,  ///< f(x) <= f(y) for all x < y
    STRICTLY_INCREASING = 1,       ///< f(x) < f(y) for all x < y
    MONOTONICALLY_DECREASING = 2,  ///< f(x) >= f(y) for all x < y
    STRICTLY_DECREASING = 3,       ///< f(x) > f(y) for all x < y
    CONSTANT = 4,                  ///< f(x) = c for c in R
    ARBITRARY = 5,
};

enum class FunctionRangeEnum
{
    NONNEGATIVE = 0,  /// f(x) >= 0 for all x
    ARBITRARY = 2,
};

template<FunctionTag F>
class FunctionSkeletonImpl
{
private:
    Index m_index;
    std::string m_name;
    VariableList m_parameters;

    // Below: add additional members if needed and initialize them in the constructor

    FunctionSkeletonImpl(Index index, std::string name, VariableList parameters);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    FunctionSkeletonImpl(const FunctionSkeletonImpl& other) = delete;
    FunctionSkeletonImpl& operator=(const FunctionSkeletonImpl& other) = delete;
    FunctionSkeletonImpl(FunctionSkeletonImpl&& other) = default;
    FunctionSkeletonImpl& operator=(FunctionSkeletonImpl&& other) = default;

    Index get_index() const;
    const std::string& get_name() const;
    const VariableList& get_parameters() const;
    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::forward_as_tuple(std::as_const(m_name), std::as_const(m_parameters)); }
};

template<FunctionTag F>
extern std::ostream& operator<<(std::ostream& out, const FunctionSkeletonImpl<F>& element);

template<FunctionTag F>
extern std::ostream& operator<<(std::ostream& out, FunctionSkeleton<F> element);

}

#endif
