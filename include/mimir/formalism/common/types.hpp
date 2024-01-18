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

#ifndef MIMIR_COMMON_TYPES_HPP_
#define MIMIR_COMMON_TYPES_HPP_

#include <loki/common/pddl/persistent_factory.hpp>

#include "../domain/domain.hpp"
#include "../problem/problem.hpp"


namespace loki {
// The segmented sizes are chosen sufficiently large to avoid
// to avoid allocations and for continuous storage.
// The values are just educated guesses based on the knowledge
// that cache line size is 64 Bytes.
// using RequirementFactory = PersistentFactory<pddl::RequirementsImpl, 100>;
// using TypeFactory = PersistentFactory<pddl::TypeImpl, 1000>;
// using VariableFactory = PersistentFactory<pddl::VariableImpl, 1000>;
// using TermFactory = PersistentFactory<pddl::TermImpl, 1000>;
// using ObjectFactory = PersistentFactory<pddl::ObjectImpl, 1000>;
// using AtomFactory = PersistentFactory<pddl::AtomImpl, 1000>;
// using LiteralFactory = PersistentFactory<pddl::LiteralImpl, 1000>;
// using ParameterFactory = PersistentFactory<pddl::ParameterImpl, 1000>;
// using PredicateFactory = PersistentFactory<pddl::PredicateImpl, 1000>;
// using FunctionExpressionFactory = PersistentFactory<pddl::FunctionExpressionImpl, 1000>;
// using FunctionFactory = PersistentFactory<pddl::FunctionImpl, 1000>;
// using FunctionSkeletonFactory = PersistentFactory<pddl::FunctionSkeletonImpl, 1000>;
// using ConditionFactory = PersistentFactory<pddl::ConditionImpl, 1000>;
// using EffectFactory = PersistentFactory<pddl::EffectImpl, 1000>;
// using ActionFactory = PersistentFactory<pddl::ActionImpl, 100>;
// using DerivedPredicateFactory = PersistentFactory<pddl::DerivedPredicateImpl, 100>;
// using OptimizationMetricFactory = PersistentFactory<pddl::OptimizationMetricImpl, 100>;
// using NumericFluentFactory = PersistentFactory<pddl::NumericFluentImpl, 1000>;
// using DomainFactory = PersistentFactory<pddl::DomainImpl, 1>;
// using ProblemFactory = PersistentFactory<pddl::ProblemImpl, 100>;


/*
/// @brief Collection of factories for the unique creation of PDDL objects.
struct CollectionOfPDDLFactories {
    RequirementFactory requirements;
    TypeFactory types;
    VariableFactory variables;
    TermFactory terms;
    ObjectFactory objects;
    AtomFactory domain_atoms;
    AtomFactory problem_atoms;  // ground atoms
    LiteralFactory domain_literals;
    LiteralFactory problem_literals;  // ground literals
    ParameterFactory parameters;
    PredicateFactory predicates;
    FunctionExpressionFactory function_expressions;
    FunctionFactory functions;
    FunctionSkeletonFactory function_skeletons;
    ConditionFactory conditions;
    EffectFactory effects;
    ActionFactory actions;
    DerivedPredicateFactory derived_predicates;
    OptimizationMetricFactory optimization_metrics;
    NumericFluentFactory numeric_fluents;
    DomainFactory domains;
    ProblemFactory problems;

    CollectionOfPDDLFactories() = default;

    // delete copy and move to avoid dangling references.
    CollectionOfPDDLFactories(const CollectionOfPDDLFactories& other) = delete;
    CollectionOfPDDLFactories& operator=(const CollectionOfPDDLFactories& other) = delete;
    CollectionOfPDDLFactories(CollectionOfPDDLFactories&& other) = delete;
    CollectionOfPDDLFactories& operator=(CollectionOfPDDLFactories&& other) = delete;
};

/// @brief Composition of factories used for parsing the domain.
///        Allows to obtain problem specific indexing schemes
///        by using problem specific factories.
///        We currently use problem specific factories for atoms and literals
struct CompositeOfPDDLFactories {
    RequirementFactory& requirements;
    TypeFactory& types;
    VariableFactory& variables;
    TermFactory& terms;
    ObjectFactory& objects;
    AtomFactory& atoms;
    LiteralFactory& literals;
    ParameterFactory& parameters;
    PredicateFactory& predicates;
    FunctionExpressionFactory& function_expressions;
    FunctionFactory& functions;
    FunctionSkeletonFactory& function_skeletons;
    ConditionFactory& conditions;
    EffectFactory& effects;
    ActionFactory& actions;
    DerivedPredicateFactory& derived_predicates;
    OptimizationMetricFactory& optimization_metrics;
    NumericFluentFactory& numeric_fluents;
    DomainFactory& domains;
    ProblemFactory& problems;
};
*/

}  // namespace mimir

#endif  // MIMIR_COMMON_TYPES_HPP_