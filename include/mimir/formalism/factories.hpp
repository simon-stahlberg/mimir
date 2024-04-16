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

#ifndef MIMIR_COMMON_FACTORIES_HPP_
#define MIMIR_COMMON_FACTORIES_HPP_

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/conditions.hpp"
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
#include "mimir/formalism/variable.hpp"

#include <loki/loki.hpp>

namespace mimir
{
// The segmented sizes are chosen sufficiently large to avoid
// to avoid allocations and for continuous storage.
// The values are just educated guesses based on the knowledge
// that cache line size is 64 Bytes.
using RequirementFactory = loki::PDDLFactory<RequirementsImpl>;
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
using AxiomFactory = loki::PDDLFactory<AxiomImpl>;
using OptimizationMetricFactory = loki::PDDLFactory<OptimizationMetricImpl>;
using NumericFluentFactory = loki::PDDLFactory<NumericFluentImpl>;
using DomainFactory = loki::PDDLFactory<DomainImpl>;
using ProblemFactory = loki::PDDLFactory<ProblemImpl>;

/// @brief Collection of factories for the unique creation of PDDL objects.
class PDDLFactories
{
private:
    RequirementFactory requirements;
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
    AxiomFactory axioms;
    OptimizationMetricFactory optimization_metrics;
    NumericFluentFactory numeric_fluents;
    DomainFactory domains;
    ProblemFactory problems;

public:
    PDDLFactories() :
        requirements(RequirementFactory(100)),
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
        axioms(AxiomFactory(100)),
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

    /// @brief Get or create requriements for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set)
    {
        return requirements.get_or_create<RequirementsImpl>(std::move(requirement_set));
    }

    /// @brief Get or create a variable for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Variable get_or_create_variable(std::string name) { return variables.get_or_create<VariableImpl>(std::move(name)); }

    /// @brief Get or create a variable term for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Term get_or_create_term_variable(Variable variable) { return terms.get_or_create<TermVariableImpl>(std::move(variable)); }

    /// @brief Get or create a object term for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Term get_or_create_term_object(Object object) { return terms.get_or_create<TermObjectImpl>(std::move(object)); }

    /// @brief Get or create an object for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Object get_or_create_object(std::string name) { return objects.get_or_create<ObjectImpl>(std::move(name)); }

    /// @brief Get or create an atom for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Atom get_or_create_atom(Predicate predicate, TermList terms) { return atoms.get_or_create<AtomImpl>(std::move(predicate), std::move(terms)); }

    /// @brief Get or create a ground atom for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundAtom get_or_create_ground_atom(Predicate predicate, ObjectList objects)
    {
        return ground_atoms.get_or_create<GroundAtomImpl>(std::move(predicate), std::move(objects));
    }

    /// @brief Get or create a literal for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Literal get_or_create_literal(bool is_negated, Atom atom) { return literals.get_or_create<LiteralImpl>(std::move(is_negated), std::move(atom)); }

    /// @brief Get or create a ground literal for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundLiteral get_or_create_ground_literal(bool is_negated, GroundAtom atom)
    {
        return ground_literals.get_or_create<GroundLiteralImpl>(std::move(is_negated), std::move(atom));
    }

    /// @brief Get or create a parameter for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Parameter get_or_create_parameter(Variable variable) { return parameters.get_or_create<ParameterImpl>(std::move(variable)); }

    /// @brief Get or create a predicate for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Predicate get_or_create_predicate(std::string name, ParameterList parameters)
    {
        return predicates.get_or_create<PredicateImpl>(std::move(name), std::move(parameters));
    }

    /// @brief Get or create a number function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_number(double number)
    {
        return function_expressions.get_or_create<FunctionExpressionNumberImpl>(number);
    }

    /// @brief Get or create a binary operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                         FunctionExpression left_function_expression,
                                                                         FunctionExpression right_function_expression)
    {
        return function_expressions.get_or_create<FunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                        std::move(left_function_expression),
                                                                                        std::move(right_function_expression));
    }

    /// @brief Get or create a multi operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions_)
    {
        return function_expressions.get_or_create<FunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions_));
    }

    /// @brief Get or create a minus function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_minus(FunctionExpression function_expression)
    {
        return function_expressions.get_or_create<FunctionExpressionMinusImpl>(std::move(function_expression));
    }

    /// @brief Get or create a function function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_function(Function function)
    {
        return function_expressions.get_or_create<FunctionExpressionFunctionImpl>(std::move(function));
    }

    /// @brief Get or create a function for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Function get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
    {
        return functions.get_or_create<FunctionImpl>(std::move(function_skeleton), std::move(terms));
    }

    /// @brief Get or create a function skeleton for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionSkeleton get_or_create_function_skeleton(std::string name, ParameterList parameters)
    {
        return function_skeletons.get_or_create<FunctionSkeletonImpl>(std::move(name), std::move(parameters));
    }

    /// @brief Get or create a literal condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_literal(Literal literal) { return conditions.get_or_create<ConditionLiteralImpl>(std::move(literal)); }

    /// @brief Get or create a and condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_and(ConditionList conditions_) { return conditions.get_or_create<ConditionAndImpl>(std::move(conditions_)); }

    /// @brief Get or create a or condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_or(ConditionList conditions_) { return conditions.get_or_create<ConditionOrImpl>(std::move(conditions_)); }

    /// @brief Get or create a or condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_not(Condition condition) { return conditions.get_or_create<ConditionNotImpl>(std::move(condition)); }

    /// @brief Get or create a imply condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_imply(Condition condition_left, Condition condition_right)
    {
        return conditions.get_or_create<ConditionImplyImpl>(std::move(condition_left), std::move(condition_right));
    }

    /// @brief Get or create an exists condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_exists(ParameterList parameters, Condition condition)
    {
        return conditions.get_or_create<ConditionExistsImpl>(std::move(parameters), std::move(condition));
    }

    /// @brief Get or create an exists condition for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Condition get_or_create_condition_forall(ParameterList parameters, Condition condition)
    {
        return conditions.get_or_create<ConditionForallImpl>(std::move(parameters), std::move(condition));
    }

    /// @brief Get or create an literal effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Effect get_or_create_effect_literal(Literal literal) { return effects.get_or_create<EffectLiteralImpl>(std::move(literal)); }

    /// @brief Get or create an and effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Effect get_or_create_effect_and(EffectList effects_) { return effects.get_or_create<EffectAndImpl>(std::move(effects_)); }

    /// @brief Get or create an numeric effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Effect get_or_create_effect_numeric(loki::AssignOperatorEnum assign_operator, Function function, FunctionExpression function_expression)
    {
        return effects.get_or_create<EffectNumericImpl>(std::move(assign_operator), std::move(function), std::move(function_expression));
    }

    /// @brief Get or create a conditional forall effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Effect get_or_create_effect_conditional_forall(ParameterList parameters, Effect effect)
    {
        return effects.get_or_create<EffectConditionalForallImpl>(std::move(parameters), std::move(effect));
    }

    /// @brief Get or create a conditional when effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Effect get_or_create_effect_conditional_when(Condition condition, Effect effect)
    {
        return effects.get_or_create<EffectConditionalWhenImpl>(std::move(condition), std::move(effect));
    }

    /// @brief Get or create an action for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Action get_or_create_action(std::string name, ParameterList parameters, LiteralList condition, std::optional<Effect> effect)
    {
        return actions.get_or_create<ActionImpl>(std::move(name), std::move(parameters), std::move(condition), std::move(effect));
    }

    /// @brief Get or create a derived predicate for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Axiom get_or_create_axiom(Literal literal, Condition condition) { return axioms.get_or_create<AxiomImpl>(std::move(literal), std::move(condition)); }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, FunctionExpression function_expression)
    {
        return optimization_metrics.get_or_create<OptimizationMetricImpl>(std::move(metric), std::move(function_expression));
    }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    NumericFluent get_or_create_numeric_fluent(Function function, double number)
    {
        return numeric_fluents.get_or_create<NumericFluentImpl>(std::move(function), std::move(number));
    }

    /// @brief Get or create a domain for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Domain get_or_create_domain(std::string name,
                                Requirements requirements,
                                ObjectList constants,
                                PredicateList predicates,
                                PredicateList derived_predicates,
                                FunctionSkeletonList functions,
                                ActionList actions,
                                AxiomList axioms)
    {
        return domains.get_or_create<DomainImpl>(std::move(name),
                                                 std::move(requirements),
                                                 std::move(constants),
                                                 std::move(predicates),
                                                 std::move(derived_predicates),
                                                 std::move(functions),
                                                 std::move(actions),
                                                 std::move(axioms));
    }

    /// @brief Get or create a problem for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Problem get_or_create_problem(Domain domain,
                                  std::string name,
                                  Requirements requirements,
                                  ObjectList objects,
                                  PredicateList derived_predicates,
                                  GroundLiteralList initial_literals,
                                  NumericFluentList numeric_fluents,
                                  std::optional<Condition> goal_condition,
                                  std::optional<OptimizationMetric> optimization_metric,
                                  AxiomList axioms)
    {
        return problems.get_or_create<ProblemImpl>(std::move(domain),
                                                   std::move(name),
                                                   std::move(requirements),
                                                   std::move(objects),
                                                   std::move(derived_predicates),
                                                   std::move(initial_literals),
                                                   std::move(numeric_fluents),
                                                   std::move(goal_condition),
                                                   std::move(optimization_metric),
                                                   std::move(axioms));
    }

    GroundAtom get_ground_atom(size_t atom_id) const { return ground_atoms.get(atom_id); }

    Object get_object(size_t object_id) const { return objects.get(object_id); }

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
