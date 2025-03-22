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

namespace mimir::formalism
{

class ProblemBuilder
{
private:
    Repositories m_repositories;

    Domain m_domain;  ///< Immutable planning domain
    std::optional<fs::path> m_filepath;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<DerivedTag> m_derived_predicates;
    GroundLiteralLists<StaticTag, FluentTag> m_initial_literals;
    GroundFunctionValueLists<StaticTag, FluentTag> m_initial_function_values;
    std::optional<GroundFunctionValue<AuxiliaryTag>> m_auxiliary_function_value;
    GroundLiteralLists<StaticTag, FluentTag, DerivedTag> m_goal_condition;
    GroundNumericConstraintList m_numeric_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;

public:
    /// @brief Create a `ProblemBuilder` for a given `Domain`.
    ///
    /// Sets the parent of the repositories to the repositories of the domain to find existing elements in there.
    /// We use canonical representations to retrieve syntactically equivalent elements from the domain in most cases,
    /// especially for the critial ones such as Type, Predicate, FunctionSkeleton, Object and all of its nested elements.
    /// We do not yet have a canonical representation for Condition, FunctionExpression, and Effect, meaning that there can be semantic duplicates.
    explicit ProblemBuilder(Domain domain);

    /// @brief Finalizes the `Problem` and returns it.
    /// The `ProblemBuilder` is in an invalid state afterwards.
    /// @return the resulting `Problem`.
    Problem get_result(Index problem_index);

    /**
     * Get and modify components of the problem.
     */

    Repositories& get_repositories();
    std::optional<fs::path>& get_filepath();
    const Domain& get_domain() const;  ///< Modifying domain is prohibitted!
    std::string& get_name();
    Requirements& get_requirements();
    ObjectList& get_objects();
    PredicateList<DerivedTag>& get_derived_predicates();
    template<IsStaticOrFluentTag P>
    GroundLiteralList<P>& get_initial_literals();
    GroundLiteralLists<StaticTag, FluentTag>& get_hana_initial_literals();
    template<IsStaticOrFluentTag F>
    GroundFunctionValueList<F>& get_initial_function_values();
    GroundFunctionValueLists<StaticTag, FluentTag>& get_hana_initial_function_values();
    std::optional<GroundFunctionValue<AuxiliaryTag>>& get_auxiliary_function_value();
    template<IsStaticOrFluentOrDerivedTag P>
    GroundLiteralList<P>& get_goal_condition();
    GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& get_hana_goal_condition();
    GroundNumericConstraintList& get_numeric_goal_condition();
    std::optional<OptimizationMetric>& get_optimization_metric();
    AxiomList& get_axioms();
};

using ProblemBuilderList = std::vector<ProblemBuilder>;
}

#endif