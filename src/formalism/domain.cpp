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

#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"

#include <algorithm>  // For std::shuffle
#include <cassert>
#include <iostream>
#include <random>  // For random number generation
#include <unordered_map>

using namespace std;

namespace mimir
{
DomainImpl::DomainImpl(size_t index,
                       std::optional<fs::path> filepath,
                       std::string name,
                       Requirements requirements,
                       ObjectList constants,
                       PredicateList<Static> static_predicates,
                       PredicateList<Fluent> fluent_predicates,
                       PredicateList<Derived> derived_predicates,
                       FunctionSkeletonList functions,
                       ActionList actions,
                       AxiomList axioms) :
    Base(index),
    m_filepath(std::move(filepath)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_constants(std::move(constants)),
    m_static_predicates(std::move(static_predicates)),
    m_fluent_predicates(std::move(fluent_predicates)),
    m_derived_predicates(std::move(derived_predicates)),
    m_functions(std::move(functions)),
    m_actions(std::move(actions)),
    m_axioms(std::move(axioms)),
    m_name_to_static_predicate(),
    m_name_to_fluent_predicate(),
    m_name_to_derived_predicate()
{
    assert(is_all_unique(m_constants));
    assert(is_all_unique(m_static_predicates));
    assert(is_all_unique(m_fluent_predicates));
    assert(is_all_unique(m_derived_predicates));
    assert(is_all_unique(m_functions));
    assert(is_all_unique(m_actions));
    assert(is_all_unique(m_axioms));

    /* Canonize. */
    std::sort(m_constants.begin(), m_constants.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_static_predicates.begin(), m_static_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_fluent_predicates.begin(), m_fluent_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_derived_predicates.begin(), m_derived_predicates.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_functions.begin(), m_functions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_actions.begin(), m_actions.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });
    std::sort(m_axioms.begin(), m_axioms.end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); });

    /* Additional */
    for (const auto& predicate : m_static_predicates)
    {
        m_name_to_static_predicate.emplace(predicate->get_name(), predicate);
    }
    for (const auto& predicate : m_fluent_predicates)
    {
        m_name_to_fluent_predicate.emplace(predicate->get_name(), predicate);
    }
    for (const auto& predicate : m_derived_predicates)
    {
        m_name_to_derived_predicate.emplace(predicate->get_name(), predicate);
    }
}

bool DomainImpl::is_structurally_equivalent_to_impl(const DomainImpl& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_requirements == other.m_requirements) && (m_constants == other.m_constants)
               && (m_static_predicates == other.m_static_predicates) && (m_fluent_predicates == other.m_fluent_predicates)
               && (m_derived_predicates == other.m_derived_predicates) && (m_functions == other.m_functions) && (m_actions == other.m_actions)
               && (m_axioms == other.m_axioms);
    }
    return true;
}

size_t DomainImpl::hash_impl() const
{
    return HashCombiner()(m_name,
                          m_requirements,
                          m_constants,
                          m_static_predicates,
                          m_fluent_predicates,
                          m_derived_predicates,
                          m_functions,
                          m_actions,
                          m_axioms);
}

void DomainImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << string(options.indent, ' ') << "(define (domain " << m_name << ")\n";
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    if (!m_requirements->get_requirements().empty())
    {
        out << string(nested_options.indent, ' ');
        m_requirements->str(out, nested_options);
        out << "\n";
    }
    if (!m_constants.empty())
    {
        out << string(nested_options.indent, ' ') << "(:constants ";
        for (size_t i = 0; i < m_constants.size(); ++i)
        {
            if (i != 0)
                out << " ";
            out << *m_constants[i];
        }
        out << ")\n";
    }
    if (!(m_static_predicates.empty() && m_fluent_predicates.empty() && !m_derived_predicates.empty()))
    {
        out << string(nested_options.indent, ' ') << "(:predicates";
        for (const auto& predicate : m_static_predicates)
        {
            out << " ";
            predicate->str(out, nested_options);
        }
        for (const auto& predicate : m_fluent_predicates)
        {
            out << " ";
            predicate->str(out, nested_options);
        }
        for (const auto& predicate : m_derived_predicates)
        {
            out << " ";
            predicate->str(out, nested_options);
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
            m_functions[i]->str(out, nested_options);
        }
    }

    for (const auto& action : m_actions)
    {
        action->str(out, nested_options, m_requirements->test(loki::RequirementEnum::ACTION_COSTS));
    }

    for (const auto& axiom : m_axioms)
    {
        axiom->str(out, nested_options);
    }

    out << std::string(options.indent, ' ') << ")";
}

const std::optional<fs::path>& DomainImpl::get_filepath() const { return m_filepath; }

const std::string& DomainImpl::get_name() const { return m_name; }

const Requirements& DomainImpl::get_requirements() const { return m_requirements; }

const ObjectList& DomainImpl::get_constants() const { return m_constants; }

template<PredicateCategory P>
const PredicateList<P>& DomainImpl::get_predicates() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_predicates;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_predicates;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_predicates;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const PredicateList<Static>& DomainImpl::get_predicates<Static>() const;
template const PredicateList<Fluent>& DomainImpl::get_predicates<Fluent>() const;
template const PredicateList<Derived>& DomainImpl::get_predicates<Derived>() const;

const FunctionSkeletonList& DomainImpl::get_functions() const { return m_functions; }

const ActionList& DomainImpl::get_actions() const { return m_actions; }

const AxiomList& DomainImpl::get_axioms() const { return m_axioms; }

template<PredicateCategory P>
const ToPredicateMap<std::string, P>& DomainImpl::get_name_to_predicate() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_name_to_static_predicate;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_name_to_fluent_predicate;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_name_to_derived_predicate;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const ToPredicateMap<std::string, Static>& DomainImpl::get_name_to_predicate<Static>() const;
template const ToPredicateMap<std::string, Fluent>& DomainImpl::get_name_to_predicate<Fluent>() const;
template const ToPredicateMap<std::string, Derived>& DomainImpl::get_name_to_predicate<Derived>() const;

}
