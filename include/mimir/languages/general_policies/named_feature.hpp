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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_NAMED_FEATURE_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_NAMED_FEATURE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::languages::general_policies
{

/// @brief `IFeature` encapsulates a `dl::Constructor` with an associated name.
/// @tparam D
template<dl::FeatureCategory D>
class NamedFeatureImpl
{
public:
    virtual ~NamedFeatureImpl() = default;

    /// @brief Get the name of the feature.
    /// @return the name of the feature.
    virtual const std::string& get_name() const = 0;

    /// @brief Get the underlying feature.
    /// @return  the underlying feature.
    virtual dl::Constructor<D> get_feature() const = 0;
};

}

#endif