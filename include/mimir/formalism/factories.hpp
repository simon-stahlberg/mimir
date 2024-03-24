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

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/conditions.hpp"
#include "mimir/formalism/derived_predicate.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/type.hpp"
#include "mimir/formalism/variable.hpp"

#include <loki/pddl/factory.hpp>

namespace mimir
{
// The segmented sizes are chosen sufficiently large to avoid
// to avoid allocations and for continuous storage.
// The values are just educated guesses based on the knowledge
// that cache line size is 64 Bytes.
using RequirementFactory = loki::PDDLFactory<RequirementsImpl>;
using TypeFactory = loki::PDDLFactory<TypeImpl>;
using VariableFactory = loki::PDDLFactory<VariableImpl>;
using TermFactory = loki::PDDLFactory<TermImpl>;
using ObjectFactory = loki::PDDLFactory<ObjectImpl>;
using AtomFactory = loki::PDDLFactory<AtomImpl>;
using GroundAtomFactory = loki::PDDLFactory<GroundAtomImpl>;
using LiteralFactory = loki::PDDLFactory<LiteralImpl>;
using GroundLiteralFactory = loki::PDDLFactory<GroundLiteralImpl>;
using ParameterFactory = loki::PDDLFactory<ParameterImpl>;
using PredicateFactory = loki::PDDLFactory<PredicateImpl>;
using FunctionExpressionFactory = loki::PDDLFactory<FunctionExpressionImpl>;
using FunctionFactory = loki::PDDLFactory<FunctionImpl>;
using FunctionSkeletonFactory = loki::PDDLFactory<FunctionSkeletonImpl>;
using ConditionFactory = loki::PDDLFactory<ConditionImpl>;
using EffectFactory = loki::PDDLFactory<EffectImpl>;
using ActionFactory = loki::PDDLFactory<ActionImpl>;
using DerivedPredicateFactory = loki::PDDLFactory<DerivedPredicateImpl>;
using OptimizationMetricFactory = loki::PDDLFactory<OptimizationMetricImpl>;
using NumericFluentFactory = loki::PDDLFactory<NumericFluentImpl>;
using DomainFactory = loki::PDDLFactory<DomainImpl>;
using ProblemFactory = loki::PDDLFactory<ProblemImpl>;

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
    DerivedPredicateFactory derived_predicates;
    OptimizationMetricFactory optimization_metrics;
    NumericFluentFactory numeric_fluents;
    DomainFactory domains;
    ProblemFactory problems;

    PDDLFactories() :
        requirements(RequirementFactory(100)),
        types(TypeFactory(1000)),
        variables(VariableFactory(1000)),
        terms(TermFactory(1000)),
        objects(ObjectFactory(1000)),
        atoms(AtomFactory(1000)),
        ground_atoms(GroundAtomFactory(1000)),
        literals(LiteralFactory(1000)),
        ground_literals(GroundLiteralFactory(1000)),
        parameters(ParameterFactory(1000)),
        predicates(PredicateFactory(1000)),
        function_expressions(FunctionExpressionFactory(1000)),
        functions(FunctionFactory(1000)),
        function_skeletons(FunctionSkeletonFactory(1000)),
        conditions(ConditionFactory(1000)),
        effects(EffectFactory(1000)),
        actions(ActionFactory(100)),
        derived_predicates(DerivedPredicateFactory(100)),
        optimization_metrics(OptimizationMetricFactory(10)),
        numeric_fluents(NumericFluentFactory(1000)),
        domains(DomainFactory(1)),
        problems(ProblemFactory(10))
    {
    }

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
    GroundLiteral to_ground_literal(Literal literal) { return get_or_create_ground_literal(literal->is_negated(), to_ground_atom(literal->get_atom())); }

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
