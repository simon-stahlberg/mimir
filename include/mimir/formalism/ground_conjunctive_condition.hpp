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

#ifndef MIMIR_FORMALISM_GROUND_CONJUNCTIVE_CONDITION_HPP_
#define MIMIR_FORMALISM_GROUND_CONJUNCTIVE_CONDITION_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{
class GroundConjunctiveConditionImpl
{
private:
    Index m_index;
    const FlatIndexList* m_positive_static_atoms;
    const FlatIndexList* m_negative_static_atoms;
    const FlatIndexList* m_positive_fluent_atoms;
    const FlatIndexList* m_negative_fluent_atoms;
    const FlatIndexList* m_positive_derived_atoms;
    const FlatIndexList* m_negative_derived_atoms;
    GroundNumericConstraintList m_numeric_constraints;

    GroundConjunctiveConditionImpl(Index index,
                                   const FlatIndexList* positive_static_atoms,
                                   const FlatIndexList* negative_static_atoms,
                                   const FlatIndexList* positive_fluent_atoms,
                                   const FlatIndexList* negative_fluent_atoms,
                                   const FlatIndexList* positive_derived_atoms,
                                   const FlatIndexList* negative_derived_atoms,
                                   GroundNumericConstraintList numeric_constraints);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundConjunctiveConditionImpl(const GroundConjunctiveConditionImpl& other) = delete;
    GroundConjunctiveConditionImpl& operator=(const GroundConjunctiveConditionImpl& other) = delete;
    GroundConjunctiveConditionImpl(GroundConjunctiveConditionImpl&& other) = default;
    GroundConjunctiveConditionImpl& operator=(GroundConjunctiveConditionImpl&& other) = default;

    Index get_index() const;

    template<IsStaticOrFluentOrDerivedTag P>
    auto get_positive_precondition() const;

    template<IsStaticOrFluentOrDerivedTag P>
    auto get_negative_precondition() const;

    const GroundNumericConstraintList& get_numeric_constraints() const;

    auto identifying_members() const
    {
        return std::tuple(m_positive_static_atoms,
                          m_negative_static_atoms,
                          m_positive_fluent_atoms,
                          m_negative_fluent_atoms,
                          m_positive_derived_atoms,
                          m_negative_derived_atoms,
                          m_numeric_constraints);
    }
};

/**
 * Implementations
 */

template<IsStaticOrFluentOrDerivedTag P>
auto GroundConjunctiveConditionImpl::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, StaticTag>)
    {
        return m_positive_static_atoms->compressed_range();
    }
    else if constexpr (std::is_same_v<P, FluentTag>)
    {
        return m_positive_fluent_atoms->compressed_range();
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return m_positive_derived_atoms->compressed_range();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template<IsStaticOrFluentOrDerivedTag P>
auto GroundConjunctiveConditionImpl::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, StaticTag>)
    {
        return m_negative_static_atoms->compressed_range();
    }
    else if constexpr (std::is_same_v<P, FluentTag>)
    {
        return m_negative_fluent_atoms->compressed_range();
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return m_negative_derived_atoms->compressed_range();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConjunctiveCondition, const formalism::ProblemImpl&>& data);
}

#endif
