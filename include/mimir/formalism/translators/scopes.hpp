/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSLATORS_SCOPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_SCOPES_HPP_

#include "mimir/formalism/declarations.hpp"

#include <deque>
#include <memory>
#include <unordered_map>

namespace mimir
{

class Scope
{
private:
    const Scope* m_parent_scope;

    std::unordered_map<Variable, Parameter> m_variable_to_parameter;

public:
    explicit Scope(const Scope* parent_scope = nullptr) : m_parent_scope(parent_scope) {}

    std::optional<Parameter> get_parameter(const Variable& variable) const
    {
        auto it = m_variable_to_parameter.find(variable);
        if (it != m_variable_to_parameter.end())
        {
            return it->second;
        }
        if (m_parent_scope)
        {
            return m_parent_scope->get_parameter(variable);
        }
        return std::nullopt;
    }

    void insert(const Parameter& parameter)
    {
        assert(!m_variable_to_parameter.count(parameter->get_variable()));
        m_variable_to_parameter.emplace(parameter->get_variable(), parameter);
    }
};

class ScopeStack
{
private:
    std::deque<std::unique_ptr<Scope>> m_stack;

public:
    Scope& open_scope()
    {
        m_stack.empty() ? m_stack.push_back(std::make_unique<Scope>()) : m_stack.push_back(std::make_unique<Scope>(&get()));
        return get();
    }

    void close_scope()
    {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }

    Scope& get() { return *m_stack.back(); }
};
}

#endif
