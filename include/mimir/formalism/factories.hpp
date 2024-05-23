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
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <loki/loki.hpp>
#include <ranges>

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
using StaticAtomFactory = loki::PDDLFactory<AtomImpl<StaticPredicateImpl>>;
using FluentAtomFactory = loki::PDDLFactory<AtomImpl<FluentPredicateImpl>>;
using StaticGroundAtomFactory = loki::PDDLFactory<GroundAtomImpl<StaticPredicateImpl>>;
using FluentGroundAtomFactory = loki::PDDLFactory<GroundAtomImpl<FluentPredicateImpl>>;
using StaticLiteralFactory = loki::PDDLFactory<LiteralImpl<StaticPredicateImpl>>;
using FluentLiteralFactory = loki::PDDLFactory<LiteralImpl<FluentPredicateImpl>>;
using StaticGroundLiteralFactory = loki::PDDLFactory<GroundLiteralImpl<StaticPredicateImpl>>;
using FluentGroundLiteralFactory = loki::PDDLFactory<GroundLiteralImpl<FluentPredicateImpl>>;
using StaticPredicateFactory = loki::PDDLFactory<StaticPredicateImpl>;
using FluentPredicateFactory = loki::PDDLFactory<FluentPredicateImpl>;
using FunctionExpressionFactory = loki::PDDLFactory<FunctionExpressionImpl>;
using GroundFunctionExpressionFactory = loki::PDDLFactory<GroundFunctionExpressionImpl>;
using FunctionFactory = loki::PDDLFactory<FunctionImpl>;
using GroundFunctionFactory = loki::PDDLFactory<GroundFunctionImpl>;
using FunctionSkeletonFactory = loki::PDDLFactory<FunctionSkeletonImpl>;
using EffectSimpleFactory = loki::PDDLFactory<EffectSimpleImpl>;
using EffectConditionalSimpleFactory = loki::PDDLFactory<EffectConditionalImpl>;
using EffectUniversalConditionalSimpleFactory = loki::PDDLFactory<EffectUniversalImpl>;
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
    StaticAtomFactory static_atoms;
    FluentAtomFactory fluent_atoms;
    StaticGroundAtomFactory static_ground_atoms;
    FluentGroundAtomFactory fluent_ground_atoms;
    StaticLiteralFactory static_literals;
    FluentLiteralFactory fluent_literals;
    StaticGroundLiteralFactory static_ground_literals;
    FluentGroundLiteralFactory fluent_ground_literals;
    StaticPredicateFactory static_predicates;
    FluentPredicateFactory fluent_predicates;
    FunctionExpressionFactory function_expressions;
    GroundFunctionExpressionFactory ground_function_expressions;
    FunctionFactory functions;
    GroundFunctionFactory ground_functions;
    FunctionSkeletonFactory function_skeletons;
    EffectSimpleFactory simple_effects;
    EffectConditionalSimpleFactory conditional_effects;
    EffectUniversalConditionalSimpleFactory universal_effects;
    ActionFactory actions;
    AxiomFactory axioms;
    OptimizationMetricFactory optimization_metrics;
    NumericFluentFactory numeric_fluents;
    DomainFactory domains;
    ProblemFactory problems;

    // TODO: provide more efficient grounding tables for arity 0, 1
    // that do not store the actual binding but instead compute a perfect hash value.
    std::vector<GroundingTable<GroundLiteral<FluentPredicateImpl>>> m_groundings_by_fluent_literal;
    std::vector<GroundingTable<GroundLiteral<StaticPredicateImpl>>> m_groundings_by_static_literal;

    template<IsPredicate P>
    GroundLiteral<P> ground_literal_generic(const Literal<P> literal, const ObjectList& binding, std::vector<GroundingTable<GroundLiteral<P>>>& grounding_table)
    {
        /* 1. Check if grounding is cached */
        const auto literal_id = literal->get_identifier();
        if (literal_id >= grounding_table.size())
        {
            grounding_table.resize(literal_id + 1);
        }

        auto& groundings = grounding_table[literal_id];

        // binding.size() might actually be greater than literal->get_predicate()->get_arity()
        // due to objects affecting variables in other literals of the same formula.
        // This results in duplicate entries in the table.
        // I tried some things but doing extra work here before the test is much slower.

        const auto it = groundings.find(binding);
        if (it != groundings.end())
        {
            return it->second;
        }

        /* 2. Ground the literal */

        ObjectList grounded_terms;
        ground_variables(literal->get_atom()->get_terms(), binding, grounded_terms);
        auto grounded_atom = create_ground_atom(literal->get_atom()->get_predicate(), std::move(grounded_terms));
        auto grounded_literal = get_or_create_ground_literal(literal->is_negated(), grounded_atom);

        /* 3. Insert to groundings table */

        groundings.emplace(ObjectList(binding), GroundLiteral(grounded_literal));

        /* 4. Return the resulting ground literal */

        return grounded_literal;
    }

public:
    PDDLFactories() :
        requirements(RequirementFactory(100)),
        variables(VariableFactory(1000)),
        terms(TermFactory(1000)),
        objects(ObjectFactory(1000)),
        static_atoms(StaticAtomFactory(1000)),
        fluent_atoms(FluentAtomFactory(1000)),
        static_ground_atoms(StaticGroundAtomFactory(1000)),
        fluent_ground_atoms(FluentGroundAtomFactory(1000)),
        static_literals(StaticLiteralFactory(1000)),
        fluent_literals(FluentLiteralFactory(1000)),
        static_ground_literals(StaticGroundLiteralFactory(1000)),
        fluent_ground_literals(FluentGroundLiteralFactory(1000)),
        static_predicates(StaticPredicateFactory(1000)),
        fluent_predicates(FluentPredicateFactory(1000)),
        function_expressions(FunctionExpressionFactory(1000)),
        ground_function_expressions(GroundFunctionExpressionFactory(1000)),
        functions(FunctionFactory(1000)),
        ground_functions(GroundFunctionFactory(1000)),
        function_skeletons(FunctionSkeletonFactory(1000)),
        simple_effects(EffectSimpleFactory(1000)),
        conditional_effects(EffectConditionalSimpleFactory(1000)),
        universal_effects(EffectUniversalConditionalSimpleFactory(1000)),
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
    Variable get_or_create_variable(std::string name, size_t parameter_index)
    {
        return variables.get_or_create<VariableImpl>(std::move(name), std::move(parameter_index));
    }

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

    Atom<StaticPredicateImpl> get_or_create_static_atom(StaticPredicate predicate, TermList terms)
    {
        return static_atoms.get_or_create<AtomImpl<StaticPredicateImpl>>(std::move(predicate), std::move(terms));
    }
    Atom<FluentPredicateImpl> get_or_create_fluent_atom(FluentPredicate predicate, TermList terms)
    {
        return fluent_atoms.get_or_create<AtomImpl<FluentPredicateImpl>>(std::move(predicate), std::move(terms));
    }

    GroundAtom<StaticPredicateImpl> get_or_create_static_ground_atom(StaticPredicate predicate, ObjectList objects)
    {
        return static_ground_atoms.get_or_create<GroundAtomImpl<StaticPredicateImpl>>(std::move(predicate), std::move(objects));
    }
    GroundAtom<FluentPredicateImpl> get_or_create_fluent_ground_atom(FluentPredicate predicate, ObjectList objects)
    {
        return fluent_ground_atoms.get_or_create<GroundAtomImpl<FluentPredicateImpl>>(std::move(predicate), std::move(objects));
    }

    Literal<StaticPredicateImpl> get_or_create_static_literal(bool is_negated, Atom<StaticPredicateImpl> atom)
    {
        return static_literals.get_or_create<LiteralImpl<StaticPredicateImpl>>(is_negated, std::move(atom));
    }
    Literal<FluentPredicateImpl> get_or_create_fluent_literal(bool is_negated, Atom<StaticPredicateImpl> atom)
    {
        return fluent_literals.get_or_create<LiteralImpl<FluentPredicateImpl>>(is_negated, std::move(atom));
    }

    GroundLiteral<StaticPredicateImpl> get_or_create_static_ground_literal(bool is_negated, GroundAtom<StaticPredicateImpl> atom)
    {
        return static_ground_literals.get_or_create<GroundLiteralImpl<StaticPredicateImpl>>(is_negated, std::move(atom));
    }
    GroundLiteral<FluentPredicateImpl> get_or_create_fluent_ground_literal(bool is_negated, GroundAtom<StaticPredicateImpl> atom)
    {
        return fluent_ground_literals.get_or_create<GroundLiteralImpl<FluentPredicateImpl>>(is_negated, std::move(atom));
    }

    StaticPredicate get_or_create_static_predicate(std::string name, VariableList parameters)
    {
        return static_predicates.get_or_create<StaticPredicateImpl>(name, std::move(parameters));
    }
    FluentPredicate get_or_create_fluent_predicate(std::string name, VariableList parameters)
    {
        return fluent_predicates.get_or_create<FluentPredicateImpl>(name, std::move(parameters));
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

    /// @brief Get or create a number function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_number(double number)
    {
        return ground_function_expressions.get_or_create<GroundFunctionExpressionNumberImpl>(number);
    }

    /// @brief Get or create a binary operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                      GroundFunctionExpression left_function_expression,
                                                                                      GroundFunctionExpression right_function_expression)
    {
        return ground_function_expressions.get_or_create<GroundFunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                                     std::move(left_function_expression),
                                                                                                     std::move(right_function_expression));
    }

    /// @brief Get or create a multi operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions_)
    {
        return ground_function_expressions.get_or_create<GroundFunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions_));
    }

    /// @brief Get or create a minus function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
    {
        return ground_function_expressions.get_or_create<GroundFunctionExpressionMinusImpl>(std::move(function_expression));
    }

    /// @brief Get or create a function function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_function(GroundFunction function)
    {
        return ground_function_expressions.get_or_create<GroundFunctionExpressionFunctionImpl>(std::move(function));
    }

    /// @brief Get or create a function for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Function get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
    {
        return functions.get_or_create<FunctionImpl>(std::move(function_skeleton), std::move(terms));
    }

    /// @brief Get or create a function for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunction get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects)
    {
        return ground_functions.get_or_create<GroundFunctionImpl>(std::move(function_skeleton), std::move(objects));
    }

    /// @brief Get or create a function skeleton for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionSkeleton get_or_create_function_skeleton(std::string name, VariableList parameters)
    {
        return function_skeletons.get_or_create<FunctionSkeletonImpl>(std::move(name), std::move(parameters));
    }

    /// @brief Get or create a simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectSimple get_or_create_simple_effect(Literal<FluentPredicateImpl> effect) { return simple_effects.get_or_create<EffectSimpleImpl>(std::move(effect)); }

    /// @brief Get or create a conditional simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectConditional get_or_create_conditional_effect(LiteralList<StaticPredicateImpl> static_condition,
                                                       LiteralList<FluentPredicateImpl> fluent_condition,
                                                       Literal<FluentPredicateImpl> effect)
    {
        return conditional_effects.get_or_create<EffectConditionalImpl>(std::move(static_condition), std::move(fluent_condition), std::move(effect));
    }

    /// @brief Get or create a universal conditional simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectUniversal get_or_create_universal_effect(VariableList parameters,
                                                   LiteralList<StaticPredicateImpl> static_condition,
                                                   LiteralList<FluentPredicateImpl> fluent_condition,
                                                   Literal<FluentPredicateImpl> effect)
    {
        return universal_effects.get_or_create<EffectUniversalImpl>(std::move(parameters),
                                                                    std::move(static_condition),
                                                                    std::move(fluent_condition),
                                                                    std::move(effect));
    }

    /// @brief Get or create an action for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Action get_or_create_action(std::string name,
                                size_t original_arity,
                                VariableList parameters,
                                LiteralList<StaticPredicateImpl> static_conditions,
                                LiteralList<FluentPredicateImpl> fluent_conditions,
                                EffectSimpleList simple_effects,
                                EffectConditionalList conditional_effects,
                                EffectUniversalList universal_effects,
                                FunctionExpression function_expression)
    {
        return actions.get_or_create<ActionImpl>(std::move(name),
                                                 std::move(original_arity),
                                                 std::move(parameters),
                                                 std::move(static_conditions),
                                                 std::move(fluent_conditions),
                                                 std::move(simple_effects),
                                                 std::move(conditional_effects),
                                                 std::move(universal_effects),
                                                 std::move(function_expression));
    }

    /// @brief Get or create a derived predicate for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Axiom get_or_create_axiom(VariableList parameters,
                              Literal<FluentPredicateImpl> literal,
                              LiteralList<StaticPredicateImpl> static_condition,
                              LiteralList<FluentPredicateImpl> fluent_condition)
    {
        return axioms.get_or_create<AxiomImpl>(std::move(parameters), std::move(literal), std::move(static_condition), std::move(fluent_condition));
    }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
    {
        return optimization_metrics.get_or_create<OptimizationMetricImpl>(std::move(metric), std::move(function_expression));
    }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    NumericFluent get_or_create_numeric_fluent(GroundFunction function, double number)
    {
        return numeric_fluents.get_or_create<NumericFluentImpl>(std::move(function), std::move(number));
    }

    /// @brief Get or create a domain for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Domain get_or_create_domain(std::string name,
                                Requirements requirements,
                                ObjectList constants,
                                StaticPredicateList static_predicates,
                                FluentPredicateList fluent_predicates,
                                FluentPredicateList derived_predicates,
                                FunctionSkeletonList functions,
                                ActionList actions,
                                AxiomList axioms)
    {
        return domains.get_or_create<DomainImpl>(std::move(name),
                                                 std::move(requirements),
                                                 std::move(constants),
                                                 std::move(static_predicates),
                                                 std::move(fluent_predicates),
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
                                  FluentPredicateList derived_predicates,
                                  GroundLiteralList<StaticPredicateImpl> static_initial_literals,
                                  GroundLiteralList<FluentPredicateImpl> fluent_initial_literals,
                                  NumericFluentList numeric_fluents,
                                  GroundLiteralList<FluentPredicateImpl> goal_condition,
                                  std::optional<OptimizationMetric> optimization_metric,
                                  AxiomList axioms)
    {
        return problems.get_or_create<ProblemImpl>(std::move(domain),
                                                   std::move(name),
                                                   std::move(requirements),
                                                   std::move(objects),
                                                   std::move(derived_predicates),
                                                   std::move(static_initial_literals),
                                                   std::move(fluent_initial_literals),
                                                   std::move(numeric_fluents),
                                                   std::move(goal_condition),
                                                   std::move(optimization_metric),
                                                   std::move(axioms));
    }

    /* Accessors */

    // GroundAtom
    GroundAtom<FluentPredicateImpl> get_ground_atom(size_t atom_id) const { return fluent_ground_atoms.get(atom_id); }

    const FluentGroundAtomFactory& get_ground_atoms() const { return fluent_ground_atoms; }

    template<std::ranges::forward_range Iterable>
    void get_ground_atoms_from_ids(const Iterable& atom_ids, GroundAtomList<FluentPredicateImpl>& out_ground_atoms) const
    {
        out_ground_atoms.clear();

        for (const auto& atom_id : atom_ids)
        {
            out_ground_atoms.push_back(get_ground_atom(atom_id));
        }
    }

    template<std::ranges::forward_range Iterable>
    GroundAtomList<FluentPredicateImpl> get_fluent_ground_atoms_from_ids(const Iterable& atom_ids) const
    {
        auto result = GroundAtomList {};
        get_ground_atoms_from_ids(atom_ids, result);
        return result;
    }

    // Static GroundAtom
    GroundAtom<StaticPredicateImpl> get_static_ground_atom(size_t atom_id) const { return static_ground_atoms.get(atom_id); }

    const StaticGroundAtomFactory& get_static_ground_atoms() const { return static_ground_atoms; }

    template<std::ranges::forward_range Iterable>
    void get_static_ground_atoms_from_ids(const Iterable& atom_ids, GroundAtomList<StaticPredicateImpl>& out_ground_atoms) const
    {
        out_ground_atoms.clear();

        for (const auto& atom_id : atom_ids)
        {
            out_ground_atoms.push_back(get_static_ground_atom(atom_id));
        }
    }

    template<std::ranges::forward_range Iterable>
    GroundAtomList<StaticPredicateImpl> get_static_ground_atoms_from_ids(const Iterable& atom_ids) const
    {
        auto result = GroundAtomList {};
        get_static_ground_atoms_from_ids(atom_ids, result);
        return result;
    }

    // Object
    template<std::ranges::forward_range Iterable>
    void get_objects_from_ids(const Iterable& object_ids, ObjectList& out_objects) const
    {
        out_objects.clear();

        for (const auto& object_id : object_ids)
        {
            out_objects.push_back(get_object(object_id));
        }
    }

    template<std::ranges::forward_range Iterable>
    ObjectList get_objects_from_ids(const Iterable& object_ids) const
    {
        auto result = ObjectList {};
        get_objects_from_ids(object_ids, result);
        return result;
    }

    Object get_object(size_t object_id) const { return objects.get(object_id); }

    /* Grounding */

    void ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
    {
        out_terms.clear();

        for (const auto& term : terms)
        {
            std::visit(
                [&](const auto& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, TermObjectImpl>)
                    {
                        out_terms.emplace_back(arg.get_object());
                    }
                    else if constexpr (std::is_same_v<T, TermVariableImpl>)
                    {
                        assert(arg.get_variable()->get_parameter_index() < binding.size());
                        out_terms.emplace_back(binding[arg.get_variable()->get_parameter_index()]);
                    }
                },
                *term);
        }
    }

    GroundLiteral<FluentPredicateImpl> ground_fluent_literal(const Literal<FluentPredicateImpl> literal, const ObjectList& binding)
    {
        return ground_literal_generic(literal, binding, m_groundings_by_fluent_literal);
    }

    GroundLiteral<StaticPredicateImpl> ground_static_literal(const Literal<StaticPredicateImpl> literal, const ObjectList& binding)
    {
        return ground_literal_generic(literal, binding, m_groundings_by_static_literal);
    }
};
}

#endif
