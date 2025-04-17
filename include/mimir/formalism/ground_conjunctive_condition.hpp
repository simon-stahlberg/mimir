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
class GroundConjunctiveCondition
{
private:
    FlatExternalPtr<const FlatIndexList> m_positive_static_atoms = nullptr;
    FlatExternalPtr<const FlatIndexList> m_negative_static_atoms = nullptr;
    FlatExternalPtr<const FlatIndexList> m_positive_fluent_atoms = nullptr;
    FlatExternalPtr<const FlatIndexList> m_negative_fluent_atoms = nullptr;
    FlatExternalPtr<const FlatIndexList> m_positive_derived_atoms = nullptr;
    FlatExternalPtr<const FlatIndexList> m_negative_derived_atoms = nullptr;
    FlatExternalPtrList<const GroundNumericConstraintImpl> m_numeric_constraints = FlatExternalPtrList<const GroundNumericConstraintImpl>();

public:
    using FormalismEntity = void;

    template<IsStaticOrFluentOrDerivedTag P>
    auto& get_positive_precondition_ptr()
    {
        if constexpr (std::is_same_v<P, StaticTag>)
        {
            return m_positive_static_atoms;
        }
        else if constexpr (std::is_same_v<P, FluentTag>)
        {
            return m_positive_fluent_atoms;
        }
        else if constexpr (std::is_same_v<P, DerivedTag>)
        {
            return m_positive_derived_atoms;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
        }
    }

    template<IsStaticOrFluentOrDerivedTag P>
    auto get_positive_precondition() const
    {
        if constexpr (std::is_same_v<P, StaticTag>)
        {
            assert(std::is_sorted(m_positive_static_atoms->compressed_begin(), m_positive_static_atoms->compressed_end()));
            return m_positive_static_atoms->compressed_range();
        }
        else if constexpr (std::is_same_v<P, FluentTag>)
        {
            assert(std::is_sorted(m_positive_fluent_atoms->compressed_begin(), m_positive_fluent_atoms->compressed_end()));
            return m_positive_fluent_atoms->compressed_range();
        }
        else if constexpr (std::is_same_v<P, DerivedTag>)
        {
            assert(std::is_sorted(m_positive_derived_atoms->compressed_begin(), m_positive_derived_atoms->compressed_end()));
            return m_positive_derived_atoms->compressed_range();
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
        }
    }

    template<IsStaticOrFluentOrDerivedTag P>
    auto& get_negative_precondition_ptr()
    {
        if constexpr (std::is_same_v<P, StaticTag>)
        {
            return m_negative_static_atoms;
        }
        else if constexpr (std::is_same_v<P, FluentTag>)
        {
            return m_negative_fluent_atoms;
        }
        else if constexpr (std::is_same_v<P, DerivedTag>)
        {
            return m_negative_derived_atoms;
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
        }
    }

    template<IsStaticOrFluentOrDerivedTag P>
    auto get_negative_precondition() const
    {
        if constexpr (std::is_same_v<P, StaticTag>)
        {
            assert(std::is_sorted(m_negative_static_atoms->compressed_begin(), m_negative_static_atoms->compressed_end()));
            return m_negative_static_atoms->compressed_range();
        }
        else if constexpr (std::is_same_v<P, FluentTag>)
        {
            assert(std::is_sorted(m_negative_fluent_atoms->compressed_begin(), m_negative_fluent_atoms->compressed_end()));
            return m_negative_fluent_atoms->compressed_range();
        }
        else if constexpr (std::is_same_v<P, DerivedTag>)
        {
            assert(std::is_sorted(m_negative_derived_atoms->compressed_begin(), m_negative_derived_atoms->compressed_end()));
            return m_negative_derived_atoms->compressed_range();
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
        }
    }

    FlatExternalPtrList<const GroundNumericConstraintImpl>& get_numeric_constraints();
    const FlatExternalPtrList<const GroundNumericConstraintImpl>& get_numeric_constraints() const;

    auto cista_members() noexcept
    {
        return std::tie(m_positive_static_atoms,
                        m_negative_static_atoms,
                        m_positive_fluent_atoms,
                        m_negative_fluent_atoms,
                        m_positive_derived_atoms,
                        m_negative_derived_atoms,
                        m_numeric_constraints);
    }
};

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
