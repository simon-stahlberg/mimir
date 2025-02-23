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

#include "mimir/formalism/problem_builder.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"

namespace mimir
{
template<typename T>
static void verify_indexing_scheme(const std::vector<const T*>& elements, const std::string& error_message)
{
    for (size_t i = 0; i < elements.size(); ++i)
    {
        if (elements[i]->get_index() != i)
        {
            throw std::runtime_error(error_message);
        }
    }
}

ProblemBuilder::ProblemBuilder(Domain domain) :
    m_repositories(),
    m_domain(domain),
    m_filepath(std::nullopt),
    m_name(""),
    m_requirements(nullptr),
    m_objects(),
    m_predicates(),
    m_initial_literals(),
    m_initial_function_values(),
    m_goal_condition(std::nullopt),
    m_optimization_metric(std::nullopt),
    m_axioms()
{
    boost::hana::for_each(m_repositories.get_hana_repositories(),
                          [&](auto&& pair)
                          {
                              auto key = boost::hana::first(pair);
                              auto& value = boost::hana::second(pair);

                              value.set_parent(boost::hana::at_key(domain->get_repositories().get_hana_repositories(), key));
                          });
}

Problem ProblemBuilder::get_result(size_t problem_index)
{
    auto problem_and_domain_objects = m_objects;
    problem_and_domain_objects.insert(problem_and_domain_objects.end(), m_domain->get_constants().begin(), m_domain->get_constants().end());
    std::sort(problem_and_domain_objects.begin(), problem_and_domain_objects.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(problem_and_domain_objects, "ProblemBuilder::get_result: problem_and_domain_objects must follow and indexing scheme");
    std::sort(m_objects.begin(), m_objects.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    auto problem_and_domain_predicates = m_predicates;
    problem_and_domain_predicates.insert(problem_and_domain_predicates.end(), m_domain->get_predicates().begin(), m_domain->get_predicates().end());
    std::sort(problem_and_domain_predicates.begin(),
              problem_and_domain_predicates.end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(problem_and_domain_predicates, "ProblemBuilder::get_result: problem_and_domain_predicates must follow and indexing scheme");
    std::sort(m_predicates.begin(), m_predicates.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    std::sort(m_initial_literals.begin(), m_initial_literals.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    std::sort(m_initial_function_values.begin(), m_initial_function_values.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    auto problem_and_domain_axioms = m_axioms;
    problem_and_domain_axioms.insert(problem_and_domain_axioms.end(), m_domain->get_axioms().begin(), m_domain->get_axioms().end());
    std::sort(m_axioms.begin(), m_axioms.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    m_requirements = (m_requirements) ? m_requirements : m_repositories.get_or_create_requirements(RequirementEnumSet { RequirementEnum::STRIPS });

    return std::make_shared<const ProblemImpl>(problem_index,
                                               std::move(m_repositories),
                                               std::move(m_filepath),
                                               std::move(m_domain),
                                               std::move(m_name),
                                               std::move(m_requirements),
                                               std::move(m_objects),
                                               std::move(problem_and_domain_objects),
                                               std::move(m_predicates),
                                               std::move(problem_and_domain_predicates),
                                               std::move(m_initial_literals),
                                               std::move(m_initial_function_values),
                                               std::move(m_goal_condition),
                                               std::move(m_optimization_metric),
                                               std::move(m_axioms),
                                               std::move(problem_and_domain_axioms));
}

Repositories& ProblemBuilder::get_repositories() { return m_repositories; }
const Domain& ProblemBuilder::get_domain() const { return m_domain; }
std::optional<fs::path>& ProblemBuilder::get_filepath() { return m_filepath; }
std::string& ProblemBuilder::get_name() { return m_name; }
Requirements& ProblemBuilder::get_requirements() { return m_requirements; }
ObjectList& ProblemBuilder::get_objects() { return m_objects; }
PredicateList& ProblemBuilder::get_predicates() { return m_predicates; }
LiteralList& ProblemBuilder::get_initial_literals() { return m_initial_literals; }
FunctionValueList& ProblemBuilder::get_function_values() { return m_initial_function_values; }
std::optional<Condition>& ProblemBuilder::get_goal_condition() { return m_goal_condition; }
std::optional<OptimizationMetric>& ProblemBuilder::get_optimization_metric() { return m_optimization_metric; }
AxiomList& ProblemBuilder::get_axioms() { return m_axioms; }
}
