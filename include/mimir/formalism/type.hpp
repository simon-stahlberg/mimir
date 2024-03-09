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

#ifndef MIMIR_FORMALISM_TYPE_HPP_
#define MIMIR_FORMALISM_TYPE_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/domain/pddl/type.hpp>
#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class TypeImpl : public loki::Base<TypeImpl>
{
private:
    std::string m_name;
    TypeList m_bases;

    // Below: add additional members if needed and initialize them in the constructor

    TypeImpl(int identifier, std::string name, TypeList bases = {});

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const TypeImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<TypeImpl>;

public:
    const std::string& get_name() const;
    const TypeList& get_bases() const;
    bool is_subtype_of(const Type type) const;
    bool is_subtype_of(const TypeList& types) const;
};

bool is_any_subtype_of(const TypeList& subtypes, const TypeList& types);
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::Type>
{
    bool operator()(const mimir::Type& left_type, const mimir::Type& right_type) const;
};

template<>
struct hash<mimir::TypeImpl>
{
    std::size_t operator()(const mimir::TypeImpl& type) const;
};
}

#endif
