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

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/variable.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {
class VariableImpl : public loki::Base<VariableImpl> {
private:
    loki::pddl::Variable external_;

    std::string m_name;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    VariableImpl(int identifier, loki::pddl::Variable external, std::string name);

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

}  // namespace mimir


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Variable>
    {
        bool operator()(const loki::pddl::Variable& left_variable, const loki::pddl::Variable& right_variable) const;
    };

    template<>
    struct hash<loki::pddl::VariableImpl>
    {
        std::size_t operator()(const loki::pddl::VariableImpl& variable) const;
    };
}  // namespace std


#endif  // MIMIR_FORMALISM_DOMAIN_VARIABLE_HPP_
