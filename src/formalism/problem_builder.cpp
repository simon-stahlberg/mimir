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
    m_derived_predicates(),
    m_initial_literals(),
    m_static_initial_function_values(),
    m_fluent_initial_function_values(),
    m_auxiliary_function_value(std::nullopt),
    m_goal_condition(),
    m_numeric_goal_condition(),
    m_optimization_metric(nullptr),
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

Problem ProblemBuilder::get_result(Index problem_index)
{
    auto problem_and_domain_objects = get_objects();
    problem_and_domain_objects.insert(problem_and_domain_objects.end(), m_domain->get_constants().begin(), m_domain->get_constants().end());
    std::sort(problem_and_domain_objects.begin(), problem_and_domain_objects.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(problem_and_domain_objects, "ProblemBuilder::get_result: problem_and_domain_objects must follow and indexing scheme");
    std::sort(get_objects().begin(), get_objects().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    auto problem_and_domain_derived_predicates = get_derived_predicates();
    problem_and_domain_derived_predicates.insert(problem_and_domain_derived_predicates.end(),
                                                 m_domain->get_predicates<Derived>().begin(),
                                                 m_domain->get_predicates<Derived>().end());
    std::sort(problem_and_domain_derived_predicates.begin(),
              problem_and_domain_derived_predicates.end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(problem_and_domain_derived_predicates,
                           "ProblemBuilder::get_result: problem_and_domain_derived_predicates must follow and indexing scheme");
    std::sort(get_derived_predicates().begin(), get_derived_predicates().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    std::sort(get_initial_literals<Static>().begin(),
              get_initial_literals<Static>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    std::sort(get_initial_literals<Fluent>().begin(),
              get_initial_literals<Fluent>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    std::sort(m_static_initial_function_values.begin(),
              m_static_initial_function_values.end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    std::sort(m_fluent_initial_function_values.begin(),
              m_fluent_initial_function_values.end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    std::sort(get_goal_condition<Static>().begin(),
              get_goal_condition<Static>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    std::sort(get_goal_condition<Fluent>().begin(),
              get_goal_condition<Fluent>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    std::sort(get_goal_condition<Derived>().begin(),
              get_goal_condition<Derived>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    std::sort(get_numeric_goal_condition().begin(),
              get_numeric_goal_condition().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    auto problem_and_domain_axioms = get_axioms();
    problem_and_domain_axioms.insert(problem_and_domain_axioms.end(), m_domain->get_axioms().begin(), m_domain->get_axioms().end());
    std::sort(get_axioms().begin(), get_axioms().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });

    m_requirements = (m_requirements) ? m_requirements : m_repositories.get_or_create_requirements(loki::RequirementEnumSet { loki::RequirementEnum::STRIPS });

    return std::shared_ptr<ProblemImpl>(new ProblemImpl(problem_index,
                                                        std::move(m_repositories),
                                                        std::move(m_filepath),
                                                        std::move(m_domain),
                                                        std::move(m_name),
                                                        std::move(m_requirements),
                                                        std::move(m_objects),
                                                        std::move(problem_and_domain_objects),
                                                        std::move(m_derived_predicates),
                                                        std::move(problem_and_domain_derived_predicates),
                                                        std::move(m_initial_literals),
                                                        std::move(m_static_initial_function_values),
                                                        std::move(m_fluent_initial_function_values),
                                                        std::move(m_auxiliary_function_value),
                                                        std::move(m_goal_condition),
                                                        std::move(m_numeric_goal_condition),
                                                        std::move(m_optimization_metric),
                                                        std::move(m_axioms),
                                                        std::move(problem_and_domain_axioms)));
}

PDDLRepositories& ProblemBuilder::get_repositories() { return m_repositories; }
std::optional<fs::path>& ProblemBuilder::get_filepath() { return m_filepath; }
const Domain& ProblemBuilder::get_domain() const { return m_domain; }
std::string& ProblemBuilder::get_name() { return m_name; }
Requirements& ProblemBuilder::get_requirements() { return m_requirements; }
ObjectList& ProblemBuilder::get_objects() { return m_objects; }
PredicateList<Derived>& ProblemBuilder::get_derived_predicates() { return m_derived_predicates; }
template<StaticOrFluent P>
GroundLiteralList<P>& ProblemBuilder::get_initial_literals()
{
    return boost::hana::at_key(m_initial_literals, boost::hana::type<P> {});
}

template GroundLiteralList<Static>& ProblemBuilder::get_initial_literals();
template GroundLiteralList<Fluent>& ProblemBuilder::get_initial_literals();

GroundLiteralLists<Static, Fluent>& ProblemBuilder::get_hana_initial_literals() { return m_initial_literals; }

template<StaticOrFluent F>
GroundFunctionValueList<F>& ProblemBuilder::get_initial_function_values()
{
    if constexpr (std::is_same_v<F, Static>)
    {
        return m_static_initial_function_values;
    }
    else if constexpr (std::is_same_v<F, Fluent>)
    {
        return m_fluent_initial_function_values;
    }
    else
    {
        static_assert(dependent_false<F>::value, "ProblemBuilder::get_initial_function_values(): Missing implementation for StaticOrFluent type.");
    }
}

template GroundFunctionValueList<Static>& ProblemBuilder::get_initial_function_values();
template GroundFunctionValueList<Fluent>& ProblemBuilder::get_initial_function_values();

std::optional<GroundFunctionValue<Auxiliary>>& ProblemBuilder::get_auxiliary_function_value() { return m_auxiliary_function_value; }
template<StaticOrFluentOrDerived P>
GroundLiteralList<P>& ProblemBuilder::get_goal_condition()
{
    return boost::hana::at_key(m_goal_condition, boost::hana::type<P> {});
}

template GroundLiteralList<Static>& ProblemBuilder::get_goal_condition();
template GroundLiteralList<Fluent>& ProblemBuilder::get_goal_condition();
template GroundLiteralList<Derived>& ProblemBuilder::get_goal_condition();

GroundLiteralLists<Static, Fluent, Derived>& ProblemBuilder::get_hana_goal_condition() { return m_goal_condition; }

GroundNumericConstraintList& ProblemBuilder::get_numeric_goal_condition() { return m_numeric_goal_condition; }
std::optional<OptimizationMetric>& ProblemBuilder::get_optimization_metric() { return m_optimization_metric; }
AxiomList& ProblemBuilder::get_axioms() { return m_axioms; }
}
