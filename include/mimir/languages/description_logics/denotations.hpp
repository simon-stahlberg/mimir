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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATIONS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_DENOTATIONS_HPP_

#include "mimir/buffering/unordered_set.h"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::languages::dl
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
struct DenotationImpl
{
};

template<>
struct DenotationImpl<ConceptTag>
{
    using DenotationType = FlatBitset;

    DenotationType m_data = DenotationType();

    DenotationType& get_data() { return m_data; }
    const DenotationType& get_data() const { return m_data; }

    bool any() const { return m_data.any(); }
    size_t count() const { return m_data.count(); }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_data())); }
};

template<>
struct DenotationImpl<RoleTag>
{
    using DenotationType = cista::offset::vector<FlatBitset>;

    DenotationType m_data = DenotationType();

    DenotationType& get_data() { return m_data; }
    const DenotationType& get_data() const { return m_data; }

    bool any() const
    {
        return std::any_of(m_data.begin(), m_data.end(), [](auto&& arg) { return arg.any(); });
    }
    size_t count() const
    {
        return std::accumulate(m_data.begin(), m_data.end(), size_t { 0 }, [](size_t sum, auto&& arg) { return sum + arg.count(); });
    }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_data())); }
};

template<>
struct DenotationImpl<BooleanTag>
{
    using DenotationType = bool;

    DenotationType m_data = DenotationType();

    DenotationType& get_data() { return m_data; }
    const DenotationType& get_data() const { return m_data; }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_data()); }
};

template<>
struct DenotationImpl<NumericalTag>
{
    using DenotationType = uint32_t;

    DenotationType m_data = DenotationType();

    DenotationType& get_data() { return m_data; }
    const DenotationType& get_data() const { return m_data; }

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_data()); }
};

/// @brief Denotation for temporary construction.
/// This stores a computed denotation for a single state.
using Denotations = HanaMappedContainer<DenotationImpl, ConceptTag, RoleTag, BooleanTag, NumericalTag>;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DenotationImplSet = mimir::buffering::UnorderedSet<DenotationImpl<D>>;

}

#endif
