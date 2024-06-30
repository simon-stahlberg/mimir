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

#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_ids.hpp"
#include "mimir/search/state.hpp"

#include <flatmemory/flatmemory.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mimir::dl
{
template<IsConceptOrRole D>
class DenotationBuilder
{
};

template<IsConceptOrRole D>
class Denotation
{
};

template<typename Tag = void>
using FlatBitsetLayout = flatmemory::Bitset<uint32_t, Tag>;
template<typename Tag = void>
using FlatBitsetBuilder = flatmemory::Builder<FlatBitsetLayout<Tag>>;
template<typename Tag = void>
using FlatBitset = flatmemory::ConstView<FlatBitsetLayout<Tag>>;

using FlatConceptDenotationLayout = FlatBitsetLayout<Concept>;
using FlatConceptDenotationBuilder = flatmemory::Builder<FlatConceptDenotationLayout>;
using FlatConceptDenotation = flatmemory::ConstView<FlatConceptDenotationLayout>;

template<>
class DenotationBuilder<Concept>
{
private:
    FlatConceptDenotationBuilder m_builder;

public:
    explicit DenotationBuilder() {}

    /**
     * Getters
     */

    FlatBitsetBuilder<Concept>& get_flatmemory_builder() { return m_builder; }
    const FlatBitsetBuilder<Concept>& get_flatmemory_builder() const { return m_builder; }

    FlatBitsetBuilder<Concept>& get_bitset() { return m_builder; }
};

template<>
class Denotation<Concept>
{
private:
    FlatConceptDenotation m_view;

public:
    using FlatDenotationLayoutType = FlatConceptDenotationLayout;
    using FlatDenotationBuilderType = FlatConceptDenotationBuilder;
    using FlatDenotationType = FlatConceptDenotation;
    using FlatDenotationSetType = flatmemory::UnorderedSet<FlatDenotationLayoutType>;

    explicit Denotation(FlatConceptDenotation view) : m_view(view) {}

    /**
     * Getters
     */

    FlatBitset<Concept> get_bitset() const { return m_view; }
};

template<typename Tag = void>
using FlatBitsetListLayout = flatmemory::Vector<flatmemory::Bitset<uint32_t, Tag>>;
template<typename Tag = void>
using FlatBitsetListBuilder = flatmemory::Builder<FlatBitsetListLayout<Tag>>;
template<typename Tag = void>
using FlatBitsetList = flatmemory::ConstView<FlatBitsetListLayout<Tag>>;

using FlatRoleDenotationLayout = FlatBitsetListLayout<Role>;
using FlatRoleDenotationBuilder = flatmemory::Builder<FlatRoleDenotationLayout>;
using FlatRoleDenotation = flatmemory::ConstView<FlatRoleDenotationLayout>;

template<>
class DenotationBuilder<Role>
{
private:
    FlatRoleDenotationBuilder m_builder;

public:
    explicit DenotationBuilder(size_t num_objects) : m_builder(FlatRoleDenotationBuilder(num_objects)) {}

    /**
     * Getters
     */

    FlatBitsetListBuilder<Role>& get_flatmemory_builder() { return m_builder; }
    const FlatBitsetListBuilder<Role>& get_flatmemory_builder() const { return m_builder; }

    FlatBitsetListBuilder<Role>& get_bitsets() { return m_builder; }
};

template<>
class Denotation<Role>
{
private:
    FlatRoleDenotation m_view;

public:
    using FlatDenotationLayoutType = FlatRoleDenotationLayout;
    using FlatDenotationBuilderType = FlatRoleDenotationBuilder;
    using FlatDenotationType = FlatRoleDenotation;
    using FlatDenotationSetType = flatmemory::UnorderedSet<FlatRoleDenotationLayout>;

    explicit Denotation(FlatRoleDenotation view) : m_view(view) {}

    /**
     * Getters
     */

    FlatBitsetList<Role> get_bitsets() const { return m_view; }
};

}

#endif
