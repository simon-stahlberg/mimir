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

#ifndef MIMIR_FORMALISM_DOMAIN_VARIABLE_HPP_
#define MIMIR_FORMALISM_DOMAIN_VARIABLE_HPP_

#include "declarations.hpp"

#include <loki/domain/pddl/variable.hpp>
#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class VariableImpl : public loki::Base<VariableImpl>
{
private:
    std::string m_name;

    // Below: add additional members if needed and initialize them in the constructor

    VariableImpl(int identifier, std::string name);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const VariableImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<VariableImpl>;

public:
    const std::string& get_name() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::Variable>
{
    bool operator()(const mimir::Variable& left_variable, const mimir::Variable& right_variable) const;
};

template<>
struct hash<mimir::VariableImpl>
{
    std::size_t operator()(const mimir::VariableImpl& variable) const;
};
}

#endif
