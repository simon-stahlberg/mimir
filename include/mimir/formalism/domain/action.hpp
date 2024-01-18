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

#ifndef MIMIR_FORMALISM_DOMAIN_ACTION_HPP_
#define MIMIR_FORMALISM_DOMAIN_ACTION_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/domain.hpp>

#include <string>
#include <functional>
#include <optional>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {
class ActionImpl : public loki::Base<ActionImpl> {
private:
    loki::pddl::Action external_;

    std::string m_name;
    ParameterList m_parameters;
    std::optional<Condition> m_condition;
    std::optional<Effect> m_effect;
    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ActionImpl(int identifier, loki::pddl::Action external, std::string name, ParameterList parameters, std::optional<Condition> condition, std::optional<Effect> effect);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const ActionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ActionImpl>;

public:
    const std::string& get_name() const;
    const ParameterList& get_parameters() const;
    const std::optional<Condition>& get_condition() const;
    const std::optional<Effect>& get_effect() const;
};

}


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<mimir::Action>
    {
        bool operator()(const mimir::Action& left_action, const mimir::Action& right_action) const;
    };

    template<>
    struct hash<mimir::ActionImpl>
    {
        std::size_t operator()(const mimir::ActionImpl& action) const;
    };
}


#endif
