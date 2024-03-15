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

#include "mimir/formalism/domain.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/type.hpp"

#include <iostream>
#include <loki/utils/collections.hpp>
#include <loki/utils/hash.hpp>
#include <unordered_map>

using namespace std;

namespace mimir
{
DomainImpl::DomainImpl(int identifier,
                       std::string name,
                       Requirements requirements,
                       TypeList types,
                       ObjectList constants,
                       PredicateList predicates,
                       FunctionSkeletonList functions,
                       ActionList actions) :
    Base(identifier),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_types(std::move(types)),
    m_constants(std::move(constants)),
    m_predicates(std::move(predicates)),
    m_static_predicates(),
    m_fluent_predicates(),
    m_functions(std::move(functions)),
    m_actions(std::move(actions))
{
    for (const auto& predicate : m_predicates)
    {
        if (any_affects(m_actions, predicate))
        {
            m_fluent_predicates.emplace_back(predicate);
        }
        else
        {
            m_static_predicates.emplace_back(predicate);
        }
    }
}

bool DomainImpl::is_structurally_equivalent_to_impl(const DomainImpl& other) const
{
    return (m_name == other.m_name) && (m_requirements == other.m_requirements) && (loki::get_sorted_vector(m_types) == loki::get_sorted_vector(other.m_types))
           && (loki::get_sorted_vector(m_constants) == loki::get_sorted_vector(other.m_constants))
           && (loki::get_sorted_vector(m_predicates) == loki::get_sorted_vector(other.m_predicates))
           && (loki::get_sorted_vector(m_functions) == loki::get_sorted_vector(other.m_functions))
           && (loki::get_sorted_vector(m_actions) == loki::get_sorted_vector(other.m_actions));
}

size_t DomainImpl::hash_impl() const
{
    return loki::hash_combine(m_name,
                              m_requirements,
                              loki::hash_container(loki::get_sorted_vector(m_types)),
                              loki::hash_container(loki::get_sorted_vector(m_constants)),
                              loki::hash_container(loki::get_sorted_vector(m_predicates)),
                              loki::hash_container(loki::get_sorted_vector(m_functions)),
                              loki::hash_container(loki::get_sorted_vector(m_actions)));
}

void DomainImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const
{
    out << string(options.indent, ' ') << "(define (domain " << m_name << ")\n";
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    if (!m_requirements->get_requirements().empty())
    {
        out << string(nested_options.indent, ' ') << m_requirements->str() << "\n";
    }
    if (!m_types.empty())
    {
        out << string(nested_options.indent, ' ') << "(:types ";
        std::unordered_map<TypeList, TypeList, loki::hash_container_type<TypeList>> subtypes_by_parent_types;
        for (const auto& type : m_types)
        {
            subtypes_by_parent_types[type->get_bases()].push_back(type);
        }
        size_t i = 0;
        for (const auto& pair : subtypes_by_parent_types)
        {
            if (i != 0)
                out << "\n" << string(nested_options.indent, ' ');
            const auto& sub_types = pair.second;
            for (size_t i = 0; i < sub_types.size(); ++i)
            {
                if (i != 0)
                    out << " ";
                out << *sub_types[i];
            }
            out << " - ";
            const auto& types = pair.first;
            for (size_t i = 0; i < types.size(); ++i)
            {
                if (i != 0)
                    out << " ";
                out << *types[i];
            }
            ++i;
        }
        out << ")\n";
    }
    if (!m_constants.empty())
    {
        out << string(nested_options.indent, ' ') << "(:constants ";
        std::unordered_map<TypeList, ObjectList, loki::hash_container_type<TypeList>> constants_by_types;
        for (const auto& constant : m_constants)
        {
            constants_by_types[constant->get_bases()].push_back(constant);
        }
        size_t i = 0;
        for (const auto& pair : constants_by_types)
        {
            if (i != 0)
                out << "\n" << string(nested_options.indent, ' ');
            const auto& constants = pair.second;
            for (size_t i = 0; i < constants.size(); ++i)
            {
                if (i != 0)
                    out << " ";
                out << *constants[i];
            }
            if (m_requirements->test(loki::pddl::RequirementEnum::TYPING))
            {
                out << " - ";
                const auto& types = pair.first;
                for (size_t i = 0; i < types.size(); ++i)
                {
                    if (i != 0)
                        out << " ";
                    out << *types[i];
                }
            }
            ++i;
        }
        out << ")\n";
    }
    if (!m_predicates.empty())
    {
        out << string(nested_options.indent, ' ') << "(:predicates ";
        for (size_t i = 0; i < m_predicates.size(); ++i)
        {
            if (i != 0)
                out << " ";
            m_predicates[i]->str(out, nested_options, m_requirements->test(loki::pddl::RequirementEnum::TYPING));
        }
        out << ")\n";
    }
    if (!m_functions.empty())
    {
        out << string(nested_options.indent, ' ') << "(:functions ";
        for (size_t i = 0; i < m_functions.size(); ++i)
        {
            if (i != 0)
                out << " ";
            out << *m_functions[i];
        }
    }

    /*
    if (node.constraints.has_value()) {
        ss << string(nested_options.indent, ' ') << parse_text(node.constraints.value(), nested_options) << "\n";
    }
    for (size_t i = 0; i < node.structures.size(); ++i) {
        ss << string(nested_options.indent, ' ') << parse_text(node.structures[i], nested_options) << "\n";
    }
    */

    for (const auto& action : m_actions)
    {
        action->str(out, nested_options);
    }
    out << std::string(options.indent, ' ') << ")";
}

const std::string& DomainImpl::get_name() const { return m_name; }

const Requirements& DomainImpl::get_requirements() const { return m_requirements; }

const TypeList& DomainImpl::get_types() const { return m_types; }

const ObjectList& DomainImpl::get_constants() const { return m_constants; }

const PredicateList& DomainImpl::get_predicates() const { return m_predicates; }

const PredicateList& DomainImpl::get_static_predicates() const { return m_static_predicates; }

const PredicateList& DomainImpl::get_fluent_predicates() const { return m_fluent_predicates; }

const FunctionSkeletonList& DomainImpl::get_functions() const { return m_functions; }

const ActionList& DomainImpl::get_actions() const { return m_actions; }

}

namespace std
{
bool less<mimir::Domain>::operator()(const mimir::Domain& left_domain, const mimir::Domain& right_domain) const { return *left_domain < *right_domain; }

std::size_t hash<mimir::DomainImpl>::operator()(const mimir::DomainImpl& domain) const { return domain.hash(); }
}
