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

#ifndef MIMIR_COMMON_FACTORIES_HPP_
#define MIMIR_COMMON_FACTORIES_HPP_

#include "action.hpp"
#include "atom.hpp"
#include "conditions.hpp"
#include "domain.hpp"
#include "effects.hpp"
#include "function.hpp"
#include "function_expressions.hpp"
#include "function_skeleton.hpp"
#include "literal.hpp"
#include "object.hpp"
#include "parameter.hpp"
#include "predicate.hpp"
#include "requirements.hpp"
#include "term.hpp"
#include "type.hpp"
#include "variable.hpp"
#include "ground_atom.hpp"
#include "ground_literal.hpp"
#include "metric.hpp"
#include "numeric_fluent.hpp"
#include "problem.hpp"

#include <loki/common/pddl/persistent_factory.hpp>

namespace mimir
{
// The segmented sizes are chosen sufficiently large to avoid
// to avoid allocations and for continuous storage.
// The values are just educated guesses based on the knowledge
// that cache line size is 64 Bytes.
using RequirementFactory = loki::PersistentFactory<RequirementsImpl, 100>;
using TypeFactory = loki::PersistentFactory<TypeImpl, 1000>;
using VariableFactory = loki::PersistentFactory<VariableImpl, 1000>;
using TermFactory = loki::PersistentFactory<TermImpl, 1000>;
using ObjectFactory = loki::PersistentFactory<ObjectImpl, 1000>;
using AtomFactory = loki::PersistentFactory<AtomImpl, 1000>;
using GroundAtomFactory = loki::PersistentFactory<GroundAtomImpl, 1000>;
using LiteralFactory = loki::PersistentFactory<LiteralImpl, 1000>;
using GroundLiteralFactory = loki::PersistentFactory<GroundLiteralImpl, 1000>;
using ParameterFactory = loki::PersistentFactory<ParameterImpl, 1000>;
using PredicateFactory = loki::PersistentFactory<PredicateImpl, 1000>;
using FunctionExpressionFactory = loki::PersistentFactory<FunctionExpressionImpl, 1000>;
using FunctionFactory = loki::PersistentFactory<FunctionImpl, 1000>;
using FunctionSkeletonFactory = loki::PersistentFactory<FunctionSkeletonImpl, 1000>;
using ConditionFactory = loki::PersistentFactory<ConditionImpl, 1000>;
using EffectFactory = loki::PersistentFactory<EffectImpl, 1000>;
using ActionFactory = loki::PersistentFactory<ActionImpl, 100>;
using OptimizationMetricFactory = loki::PersistentFactory<OptimizationMetricImpl, 100>;
using NumericFluentFactory = loki::PersistentFactory<NumericFluentImpl, 1000>;
using DomainFactory = loki::PersistentFactory<DomainImpl, 1>;
using ProblemFactory = loki::PersistentFactory<ProblemImpl, 100>;

/// @brief Collection of factories for the unique creation of PDDL objects.
struct PDDLFactories
{
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

    /// @brief Get or create a ground atom for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundAtom get_or_create_ground_atom(Predicate predicate, ObjectList objects)
    {
        return ground_atoms.get_or_create<GroundAtomImpl>(std::move(predicate), std::move(objects));
    }

    /// @brief Get or create a ground literal for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundLiteral get_or_create_ground_literal(bool is_negated, GroundAtom atom)
    {
        return ground_literals.get_or_create<GroundLiteralImpl>(std::move(is_negated), std::move(atom));
    }

    GroundAtom to_ground_atom(Atom atom)
    {
        ObjectList terms;

        for (const auto& term : atom->get_terms())
        {
            if (const auto* object_term = std::get_if<TermObjectImpl>(term))
            {
                terms.emplace_back(object_term->get_object());
            }
            else
            {
                throw std::runtime_error("atom contains a variable");
            }
        }

        return get_or_create_ground_atom(atom->get_predicate(), terms);
    }

    /// @brief Converts an alraedy grounded Literal to type GroundLiteral.
    GroundLiteral to_ground_literal(Literal literal)
    {
        return get_or_create_ground_literal(literal->is_negated(), to_ground_atom(literal->get_atom()));
    }

    /// @brief Converts a list of already grounded Literal elements to a list of type GroundLiteral.
    void to_ground_literals(const LiteralList& literals, GroundLiteralList& out_ground_literals)
    {
        out_ground_literals.clear();

        for (const auto& literal : literals)
        {
            out_ground_literals.emplace_back(to_ground_literal(literal));
        }
    }
};
}

#endif
