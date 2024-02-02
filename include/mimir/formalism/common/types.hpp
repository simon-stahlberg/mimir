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

#include "../domain/action.hpp"
#include "../domain/atom.hpp"
#include "../domain/conditions.hpp"
#include "../domain/domain.hpp"
#include "../domain/effects.hpp"
#include "../domain/function_expressions.hpp"
#include "../domain/function_skeleton.hpp"
#include "../domain/function.hpp"
#include "../domain/literal.hpp"
#include "../domain/object.hpp"
#include "../domain/parameter.hpp"
#include "../domain/predicate.hpp"
#include "../domain/requirements.hpp"
#include "../domain/term.hpp"
#include "../domain/type.hpp"
#include "../domain/variable.hpp"
#include "../problem/ground_atom.hpp"
#include "../problem/ground_literal.hpp"
#include "../problem/problem.hpp"
#include "../problem/metric.hpp"
#include "../problem/numeric_fluent.hpp"


namespace mimir 
{
    // The segmented sizes are chosen sufficiently large to avoid
    // to avoid allocations and for continuous storage.
    // The values are just educated guesses based on the knowledge
    // that cache line size is 64 Bytes.
    using RequirementFactory =        loki::PersistentFactory<RequirementsImpl, 100>;
    using TypeFactory =               loki::PersistentFactory<TypeImpl, 1000>;
    using VariableFactory =           loki::PersistentFactory<VariableImpl, 1000>;
    using TermFactory =               loki::PersistentFactory<TermImpl, 1000>;
    using ObjectFactory =             loki::PersistentFactory<ObjectImpl, 1000>;
    using AtomFactory =               loki::PersistentFactory<AtomImpl, 1000>;
    using GroundAtomFactory =         loki::PersistentFactory<GroundAtomImpl, 1000>;
    using LiteralFactory =            loki::PersistentFactory<LiteralImpl, 1000>;
    using GroundLiteralFactory =      loki::PersistentFactory<GroundLiteralImpl, 1000>;
    using ParameterFactory =          loki::PersistentFactory<ParameterImpl, 1000>;
    using PredicateFactory =          loki::PersistentFactory<PredicateImpl, 1000>;
    using FunctionExpressionFactory = loki::PersistentFactory<FunctionExpressionImpl, 1000>;
    using FunctionFactory =           loki::PersistentFactory<FunctionImpl, 1000>;
    using FunctionSkeletonFactory =   loki::PersistentFactory<FunctionSkeletonImpl, 1000>;
    using ConditionFactory =          loki::PersistentFactory<ConditionImpl, 1000>;
    using EffectFactory =             loki::PersistentFactory<EffectImpl, 1000>;
    using ActionFactory =             loki::PersistentFactory<ActionImpl, 100>;
    using OptimizationMetricFactory = loki::PersistentFactory<OptimizationMetricImpl, 100>;
    using NumericFluentFactory =      loki::PersistentFactory<NumericFluentImpl, 1000>;
    using DomainFactory =             loki::PersistentFactory<DomainImpl, 1>;
    using ProblemFactory =            loki::PersistentFactory<ProblemImpl, 100>;


    /// @brief Collection of factories for the unique creation of PDDL objects.
    struct PDDLFactories {
        RequirementFactory requirements;
        TypeFactory types;
        VariableFactory variables;
        TermFactory terms;
        ObjectFactory objects;
        AtomFactory atoms;
        GroundAtomFactory ground_atoms;
        LiteralFactory literals;
        GroundLiteralFactory ground_literals;
        ParameterFactory parameters;
        PredicateFactory predicates;
        FunctionExpressionFactory function_expressions;
        FunctionFactory functions;
        FunctionSkeletonFactory function_skeletons;
        ConditionFactory conditions;
        EffectFactory effects;
        ActionFactory actions;
        OptimizationMetricFactory optimization_metrics;
        NumericFluentFactory numeric_fluents;
        DomainFactory domains;
        ProblemFactory problems;

        PDDLFactories() = default;

        // delete copy and move to avoid dangling references.
        PDDLFactories(const PDDLFactories& other) = delete;
        PDDLFactories& operator=(const PDDLFactories& other) = delete;
        PDDLFactories(PDDLFactories&& other) = delete;
        PDDLFactories& operator=(PDDLFactories&& other) = delete;
    };
} 

#endif 
