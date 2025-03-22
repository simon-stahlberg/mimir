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

#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{

/// @brief `NamedFeatureImpl` encapsulates a `dl::Constructor` with an associated name.
/// @tparam D is the feature type.
template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class NamedFeatureImpl
{
private:
    Index m_index;
    std::string m_name;
    dl::Constructor<D> m_feature;

    NamedFeatureImpl(Index index, std::string name, dl::Constructor<D> feature);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    /// @brief Accept a `IVisitor`.
    /// @param visitor the `IVisitor`.
    void accept(IVisitor& visitor) const;

    /// @brief Get the index of the `NamedFeatureImpl`.
    /// @return the index of the `NamedFeatureImpl`.
    Index get_index() const;

    /// @brief Get the name.
    /// @return the name.
    const std::string& get_name() const;

    /// @brief Get the `dl::Constructor`.
    /// @return  the `dl::Constructor`.
    dl::Constructor<D> get_feature() const;

    auto identifying_members() const { return std::tuple(std::cref(get_name()), get_feature()); }
};

}

#endif