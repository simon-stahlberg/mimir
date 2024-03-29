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

#include "mimir/formalism/problem.hpp"

#include "loki/pddl/visitors.hpp"
#include "mimir/formalism/conditions.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/type.hpp"

#include <iostream>
#include <loki/utils/collections.hpp>
#include <loki/utils/hash.hpp>

using namespace std;

namespace mimir
{
ProblemImpl::ProblemImpl(int identifier,
                         Domain domain,
                         std::string name,
                         Requirements requirements,
                         ObjectList objects,
                         GroundLiteralList initial_literals,
                         NumericFluentList numeric_fluents,
                         Condition goal_condition,
                         std::optional<OptimizationMetric> optimization_metric) :
    Base(identifier),
    m_domain(std::move(domain)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_objects(std::move(objects)),
    m_initial_literals(std::move(initial_literals)),
    m_numeric_fluents(std::move(numeric_fluents)),
    m_goal_condition(std::move(goal_condition)),
    m_optimization_metric(std::move(optimization_metric))
{
}

bool ProblemImpl::is_structurally_equivalent_to_impl(const ProblemImpl& other) const
{
    return (m_domain == other.m_domain) && (m_name == other.m_name) && (m_requirements == other.m_requirements)
           && (get_sorted_vector(m_objects) == get_sorted_vector(other.m_objects))
           && (get_sorted_vector(m_initial_literals)) == get_sorted_vector(other.m_initial_literals) && (m_goal_condition == other.m_goal_condition)
           && (m_optimization_metric == other.m_optimization_metric);
}

size_t ProblemImpl::hash_impl() const
{
    size_t optimization_hash = (m_optimization_metric.has_value()) ? hash_combine(m_optimization_metric) : 0;
    return hash_combine(m_domain,
                        m_name,
                        m_requirements,
                        hash_container(get_sorted_vector(m_objects)),
                        hash_container(get_sorted_vector(m_initial_literals)),
                        m_goal_condition,
                        optimization_hash);
}

void ProblemImpl::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << string(options.indent, ' ') << "(define (problem " << m_name << ")\n";
    auto nested_options = loki::FormattingOptions { options.indent + options.add_indent, options.add_indent };
    out << string(nested_options.indent, ' ') << "(:domain " << m_domain->get_name() << ")\n";
    if (!m_requirements->get_requirements().empty())
    {
        out << string(nested_options.indent, ' ');
        m_requirements->str(out, nested_options);
        out << "\n";
    }

    if (!m_objects.empty())
    {
        out << string(nested_options.indent, ' ') << "(:objects ";
        std::unordered_map<TypeList, ObjectList, loki::hash_container_type<TypeList>> objects_by_types;
        for (const auto& object : m_objects)
        {
            objects_by_types[object->get_bases()].push_back(object);
        }
        size_t i = 0;
        for (const auto& [types, objects] : objects_by_types)
        {
            if (i != 0)
                out << "\n" << string(nested_options.indent, ' ');
            for (size_t i = 0; i < objects.size(); ++i)
            {
                if (i != 0)
                    out << " ";
                out << objects[i]->get_name();
            }
            if (m_requirements->test(loki::pddl::RequirementEnum::TYPING))
            {
                out << " - ";
                if (types.size() > 1)
                {
                    out << "(either ";
                    for (size_t i = 0; i < types.size(); ++i)
                    {
                        if (i != 0)
                            out << " ";
                        types[i]->get_name();
                    }
                    out << ")";
                }
                else if (types.size() == 1)
                {
                    out << types.front()->get_name();
                }
            }
            ++i;
        }
        out << ")\n";
    }

    out << string(nested_options.indent, ' ') << "(:init ";
    for (size_t i = 0; i < m_initial_literals.size(); ++i)
    {
        if (i != 0)
            out << " ";
        m_initial_literals[i]->str(out, nested_options);
    }
    for (size_t i = 0; i < m_numeric_fluents.size(); ++i)
    {
        out << " ";
        m_numeric_fluents[i]->str(out, nested_options);
    }

    out << ")\n";
    out << string(nested_options.indent, ' ') << "(:goal ";
    std::visit(loki::pddl::StringifyVisitor(out, options), *m_goal_condition);

    out << ")\n";
    if (m_optimization_metric.has_value())
    {
        out << string(nested_options.indent, ' ') << "(:metric ";
        m_optimization_metric.value()->str(out, nested_options);
        out << ")\n";
    }
    /*
    if (node.constraints.has_value()) {
        ss << string(nested_options.indent, ' ') << parse_text(node.constraints.value(), nested_options) << "\n";
    }
    */

    out << string(options.indent, ' ') << ")";
}

const Domain& ProblemImpl::get_domain() const { return m_domain; }

const std::string& ProblemImpl::get_name() const { return m_name; }

const Requirements& ProblemImpl::get_requirements() const { return m_requirements; }

const ObjectList& ProblemImpl::get_objects() const { return m_objects; }

const GroundLiteralList& ProblemImpl::get_initial_literals() const { return m_initial_literals; }

const NumericFluentList& ProblemImpl::get_numeric_fluents() const { return m_numeric_fluents; }

const Condition& ProblemImpl::get_goal_condition() const { return m_goal_condition; }

const std::optional<OptimizationMetric>& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

}
