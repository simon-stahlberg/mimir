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

#ifndef MIMIR_FORMALISM_PARAMETER_HPP_
#define MIMIR_FORMALISM_PARAMETER_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/parameter.hpp>
#include <string>

namespace loki
{
template<typename HolderType>
class PersistentFactory;
}

namespace mimir
{
class ParameterImpl : public loki::Base<ParameterImpl>
{
private:
    Variable m_variable;
    TypeList m_types;

    // Below: add additional members if needed and initialize them in the constructor

    ParameterImpl(int identifier, Variable variable, TypeList types);

    // Give access to the constructor.
    template<typename>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const ParameterImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ParameterImpl>;

public:
    /// @brief Returns a parseable string representation in the context of a domain.
    void str(std::ostringstream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const Variable& get_variable() const;
    const TypeList& get_bases() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::Parameter>
{
    bool operator()(const mimir::Parameter& left_parameter, const mimir::Parameter& right_parameter) const;
};

template<>
struct hash<mimir::ParameterImpl>
{
    std::size_t operator()(const mimir::ParameterImpl& parameter) const;
};
}

#endif
