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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_CONDITION_BASE_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_CONDITION_BASE_HPP_

#include "mimir/languages/general_policies/condition_interface.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
/// @brief `ConditionBase` encapsulates common data and functionality for concrete `ICondition`.
/// @tparam Derived_ is the derived type.
/// @tparam D is the feature type.
template<typename Derived_, dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class ConditionBase : public ICondition
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    Index m_index;
    NamedFeature<D> m_feature;

public:
    ConditionBase(Index index, NamedFeature<D> feature) : m_index(index), m_feature(feature) {}

    Index get_index() const override { return m_index; }

    NamedFeature<D> get_feature() const { return m_feature; }

    bool evaluate(dl::EvaluationContext& source_context) const override { return self().evaluate_impl(source_context); }

    void accept(IVisitor& visitor) const override { self().accept_impl(visitor); };

    auto identifying_members() const { return std::tuple(get_feature()); }
};
}

#endif