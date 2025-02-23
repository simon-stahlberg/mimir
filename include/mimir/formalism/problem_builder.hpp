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

#ifndef MIMIR_FORMALISM_PROBLEM_BUILDER_HPP_
#define MIMIR_FORMALISM_PROBLEM_BUILDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/repositories.hpp"

#include <optional>
#include <string>

namespace mimir
{

class ProblemBuilder
{
private:
    PDDLRepositories m_repositories;

    Domain m_domain;  ///< Immutable planning domain
    std::optional<fs::path> m_filepath;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<Derived> m_derived_predicates;
    GroundLiteralList<Static> m_static_initial_literals;
    GroundLiteralList<Fluent> m_fluent_initial_literals;
    GroundFunctionValueList<Static> m_static_function_values;
    GroundFunctionValueList<Fluent> m_fluent_function_values;
    std::optional<GroundFunctionValue<Auxiliary>> m_auxiliary_function_value;
    GroundLiteralList<Static> m_static_goal_condition;
    GroundLiteralList<Fluent> m_fluent_goal_condition;
    GroundLiteralList<Derived> m_derived_goal_condition;
    GroundNumericConstraintList m_numeric_goal_condition;
    OptimizationMetric m_optimization_metric;
    AxiomList m_axioms;

public:
    /// @brief Create a `ProblemBuilder` for a given `Domain`.
    ///
    /// Copies all elements from the domain repositories as external elements into the problem repositories.
    /// We use canonical representations to retrieve syntactically equivalent elements from the domain in most cases,
    /// especially for the critial ones such as Type, Predicate, FunctionSkeleton, Object and all of its nested elements.
    /// We do not yet have a canonical representation for Condition, FunctionExpression, and Effect, meaning that there can be semantic duplicates.
    ///
    /// The canonical representation is important during translation to ensure that we can decouple the domain from the problem translation.
    /// Let's assume we translate the domain first, meaning that we get new addresses for all elements and that we now translate a problem.
    /// The copy step ensures that we reuse elements of the translated domain during the problem translation.
    explicit ProblemBuilder(Domain domain);

    /// @brief Finalizes the `Problem` and returns it.
    /// The `ProblemBuilder` is in an invalid state afterwards.
    /// @return the resulting `Problem`.
    Problem get_result(size_t problem_index);

    /**
     * Get and modify components of the problem.
     */

    PDDLRepositories& get_repositories();
    std::optional<fs::path>& get_filepath();
    Domain& get_domain();
    std::string& get_name();
    Requirements& get_requirements();
    ObjectList& get_objects();
    PredicateList<Derived>& get_derived_predicates();
    GroundLiteralList<Static>& get_static_initial_literals();
    GroundLiteralList<Fluent>& get_fluent_initial_literals();
    template<StaticOrFluentTag F>
    GroundFunctionValueList<F>& get_function_values();
    std::optional<GroundFunctionValue<Auxiliary>>& get_auxiliary_function_value();
    template<StaticOrFluentOrDerived P>
    GroundLiteralList<P>& get_goal_condition();
    GroundNumericConstraintList& get_numeric_goal_condition();
    OptimizationMetric& get_optimization_metric();
    AxiomList& get_axioms();
};

using ProblemBuilderList = std::vector<ProblemBuilder>;
}

#endif