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

#include "mimir/common/hash_cista.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/languages/description_logics/constructor_category.hpp"

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

using FlatConceptDenotation = FlatBitset;

template<>
class DenotationBuilder<Concept>
{
private:
    FlatConceptDenotation m_builder;

public:
    using FlatDenotationType = FlatConceptDenotation;

    explicit DenotationBuilder() {}

    /**
     * Getters
     */

    FlatConceptDenotation& get_data() { return m_builder; }
    const FlatConceptDenotation& get_data() const { return m_builder; }
};

template<>
class Denotation<Concept>
{
private:
    const FlatConceptDenotation* m_view;

public:
    using FlatDenotationType = FlatConceptDenotation;

    explicit Denotation(const FlatConceptDenotation* view) : m_view(view) {}

    /**
     * Getters
     */

    const FlatConceptDenotation& get_bitset() const { return *m_view; }
};

using FlatRoleDenotation = cista::offset::vector<FlatBitset>;

template<>
class DenotationBuilder<Role>
{
private:
    FlatRoleDenotation m_builder;

public:
    using FlatDenotationType = FlatRoleDenotation;

    explicit DenotationBuilder(size_t num_objects) : m_builder(num_objects) {}

    /**
     * Getters
     */

    FlatRoleDenotation& get_data() { return m_builder; }
    const FlatRoleDenotation& get_data() const { return m_builder; }
};

template<>
class Denotation<Role>
{
private:
    const FlatRoleDenotation* m_view;

public:
    using FlatDenotationType = FlatRoleDenotation;

    explicit Denotation(const FlatRoleDenotation* view) : m_view(view) {}

    /**
     * Getters
     */

    const FlatRoleDenotation& get_data() const { return *m_view; }
};

}

#endif
