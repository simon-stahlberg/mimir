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

#ifndef MIMIR_FORMALISM_ACTION_HPP_
#define MIMIR_FORMALISM_ACTION_HPP_

#include "mimir/formalism/declarations.hpp"

#include <functional>
#include <loki/pddl/action.hpp>
#include <loki/pddl/factory.hpp>
#include <optional>
#include <string>

namespace mimir
{
class ActionImpl : public loki::Base<ActionImpl>
{
private:
    std::string m_name;
    ParameterList m_parameters;
    std::optional<Condition> m_condition;
    std::optional<Effect> m_effect;

    // Below: add additional members if needed and initialize them in the constructor

    ActionImpl(int identifier, std::string name, ParameterList parameters, std::optional<Condition> condition, std::optional<Effect> effect);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

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
    size_t get_arity() const;

    bool affects(Predicate predicate) const;
};

bool any_affects(const ActionList& actions, Predicate predicate);
}

#endif
