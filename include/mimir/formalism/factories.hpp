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
using VariadicPDDLFactories = loki::VariadicPDDLFactory<RequirementsImpl,
                                                        VariableImpl,
                                                        TermImpl,
                                                        ObjectImpl,
                                                        AtomImpl<Static>,
                                                        AtomImpl<Fluent>,
                                                        AtomImpl<Derived>,
                                                        GroundAtomImpl<Static>,
                                                        GroundAtomImpl<Fluent>,
                                                        GroundAtomImpl<Derived>,
                                                        LiteralImpl<Static>,
                                                        LiteralImpl<Fluent>,
                                                        LiteralImpl<Derived>,
                                                        GroundLiteralImpl<Static>,
                                                        GroundLiteralImpl<Fluent>,
                                                        GroundLiteralImpl<Derived>,
                                                        PredicateImpl<Static>,
                                                        PredicateImpl<Fluent>,
                                                        PredicateImpl<Derived>,
                                                        FunctionExpressionImpl,
                                                        GroundFunctionExpressionImpl,
                                                        FunctionImpl,
                                                        GroundFunctionImpl,
                                                        FunctionSkeletonImpl,
                                                        EffectSimpleImpl,
                                                        EffectConditionalImpl,
                                                        EffectUniversalImpl,
                                                        ActionImpl,
                                                        AxiomImpl,
                                                        OptimizationMetricImpl,
                                                        NumericFluentImpl,
                                                        DomainImpl,
                                                        ProblemImpl>;

/// @brief Collection of factories for the unique creation of PDDL objects.
///
/// TODO: we would like to use the pddl objects of the domain in the problem
class PDDLFactories
{
private:
    VariadicPDDLFactories m_factories;

    // TODO: provide more efficient grounding tables for arity 0, 1
    // that do not store the actual binding but instead compute a perfect hash value.
    std::vector<GroundingTable<GroundLiteral<Static>>> m_groundings_by_static_literal;
    std::vector<GroundingTable<GroundLiteral<Fluent>>> m_groundings_by_fluent_literal;
    std::vector<GroundingTable<GroundLiteral<Derived>>> m_groundings_by_derived_literal;

    template<PredicateCategory P>
    GroundLiteral<P>
    ground_literal_generic(const Literal<P> literal, const ObjectList& binding, std::vector<GroundingTable<GroundLiteral<P>>>& grounding_table);

public:
    PDDLFactories();

    // delete copy and allow move
    PDDLFactories(const PDDLFactories& other) = delete;
    PDDLFactories& operator=(const PDDLFactories& other) = delete;
    PDDLFactories(PDDLFactories&& other);
    PDDLFactories& operator=(PDDLFactories&& other);

    /// @brief Get or create requriements for the given parameters.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set);

    /// @brief Get or create a variable for the given parameters.
    Variable get_or_create_variable(std::string name, size_t parameter_index);

    /// @brief Get or create a variable term for the given parameters.
    Term get_or_create_term_variable(Variable variable);

    /// @brief Get or create a object term for the given parameters.
    Term get_or_create_term_object(Object object);

    /// @brief Get or create an object for the given parameters.
    Object get_or_create_object(std::string name);

    template<PredicateCategory P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms);

    template<PredicateCategory P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects);

    template<PredicateCategory P>
    Literal<P> get_or_create_literal(bool is_negated, Atom<P> atom);

    template<PredicateCategory P>
    GroundLiteral<P> get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom);

    template<PredicateCategory P>
    Predicate<P> get_or_create_predicate(std::string name, VariableList parameters);

    /// @brief Get or create a number function expression for the given parameters.
    FunctionExpression get_or_create_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    FunctionExpression get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                         FunctionExpression left_function_expression,
                                                                         FunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    FunctionExpression get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    FunctionExpression get_or_create_function_expression_minus(FunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    FunctionExpression get_or_create_function_expression_function(Function function);

    /// @brief Get or create a number function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                      GroundFunctionExpression left_function_expression,
                                                                                      GroundFunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression_function(GroundFunction function);

    /// @brief Get or create a function for the given parameters.
    Function get_or_create_function(FunctionSkeleton function_skeleton, TermList terms);

    /// @brief Get or create a function for the given parameters.
    GroundFunction get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects);

    /// @brief Get or create a function skeleton for the given parameters.
    FunctionSkeleton get_or_create_function_skeleton(std::string name, VariableList parameters);

    /// @brief Get or create a simple effect for the given parameters.
    EffectSimple get_or_create_simple_effect(Literal<Fluent> effect);

    /// @brief Get or create a conditional simple effect for the given parameters.
    EffectConditional get_or_create_conditional_effect(LiteralList<Static> static_conditions,
                                                       LiteralList<Fluent> fluent_conditions,
                                                       LiteralList<Derived> derived_conditions,
                                                       Literal<Fluent> effect);

    /// @brief Get or create a universal conditional simple effect for the given parameters.
    EffectUniversal get_or_create_universal_effect(VariableList parameters,
                                                   LiteralList<Static> static_conditions,
                                                   LiteralList<Fluent> fluent_conditions,
                                                   LiteralList<Derived> derived_conditions,
                                                   Literal<Fluent> effect);

    /// @brief Get or create an action for the given parameters.
    Action get_or_create_action(std::string name,
                                size_t original_arity,
                                VariableList parameters,
                                LiteralList<Static> static_conditions,
                                LiteralList<Fluent> fluent_conditions,
                                LiteralList<Derived> derived_conditions,
                                EffectSimpleList simple_effects,
                                EffectConditionalList conditional_effects,
                                EffectUniversalList universal_effects,
                                FunctionExpression function_expression);

    /// @brief Get or create a derived predicate for the given parameters.
    Axiom get_or_create_axiom(VariableList parameters,
                              Literal<Derived> literal,
                              LiteralList<Static> static_condition,
                              LiteralList<Fluent> fluent_condition,
                              LiteralList<Derived> derived_conditions);

    /// @brief Get or create an optimization metric for the given parameters.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression);

    /// @brief Get or create an optimization metric for the given parameters.
    NumericFluent get_or_create_numeric_fluent(GroundFunction function, double number);

    /// @brief Get or create a domain for the given parameters.
    Domain get_or_create_domain(std::optional<fs::path> filepath,
                                std::string name,
                                Requirements requirements,
                                ObjectList constants,
                                PredicateList<Static> static_predicates,
                                PredicateList<Fluent> fluent_predicates,
                                PredicateList<Derived> derived_predicates,
                                FunctionSkeletonList functions,
                                ActionList actions,
                                AxiomList axioms);

    /// @brief Get or create a problem for the given parameters.
    Problem get_or_create_problem(std::optional<fs::path> filepath,
                                  Domain domain,
                                  std::string name,
                                  Requirements requirements,
                                  ObjectList objects,
                                  PredicateList<Derived> derived_predicates,
                                  GroundLiteralList<Static> static_initial_literals,
                                  GroundLiteralList<Fluent> fluent_initial_literals,
                                  NumericFluentList numeric_fluents,
                                  GroundLiteralList<Static> static_goal_condition,
                                  GroundLiteralList<Fluent> fluent_goal_condition,
                                  GroundLiteralList<Derived> derived_goal_condition,
                                  std::optional<OptimizationMetric> optimization_metric,
                                  AxiomList axioms);

    /* Accessors */

    // Factory
    template<typename T>
    const loki::PDDLFactory<T>& get_factory() const;

    // GroundAtom
    template<PredicateCategory P>
    GroundAtom<P> get_ground_atom(size_t atom_id) const;

    template<PredicateCategory P, std::ranges::forward_range Iterable>
    void get_ground_atoms_from_ids(const Iterable& atom_ids, GroundAtomList<P>& out_ground_atoms) const;

    template<PredicateCategory P, std::ranges::forward_range Iterable>
    GroundAtomList<P> get_ground_atoms_from_ids(const Iterable& atom_ids) const;

    // Object
    Object get_object(size_t object_id) const;

    template<std::ranges::forward_range Iterable>
    void get_objects_from_ids(const Iterable& object_ids, ObjectList& out_objects) const;

    template<std::ranges::forward_range Iterable>
    ObjectList get_objects_from_ids(const Iterable& object_ids) const;

    /* Grounding */

    void ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms);

    GroundLiteral<Static> ground_literal(const Literal<Static> literal, const ObjectList& binding);

    GroundLiteral<Fluent> ground_literal(const Literal<Fluent> literal, const ObjectList& binding);

    GroundLiteral<Derived> ground_literal(const Literal<Derived> literal, const ObjectList& binding);

    template<PredicateCategory P>
    void ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                FlatBitsetBuilder<P>& ref_positive_bitset,
                                FlatBitsetBuilder<P>& ref_negative_bitset,
                                const ObjectList& binding);

    template<PredicateCategory P>
    void ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                FlatIndexListBuilder& ref_positive_indices,
                                FlatIndexListBuilder& ref_negative_indices,
                                const ObjectList& binding);
};

/**
 * Implementations
 */

template<PredicateCategory P, std::ranges::forward_range Iterable>
void PDDLFactories::get_ground_atoms_from_ids(const Iterable& atom_ids, GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();

    for (const auto& atom_id : atom_ids)
    {
        out_ground_atoms.push_back(get_ground_atom<P>(atom_id));
    }
}

template<PredicateCategory P, std::ranges::forward_range Iterable>
GroundAtomList<P> PDDLFactories::get_ground_atoms_from_ids(const Iterable& atom_ids) const
{
    auto result = GroundAtomList<P> {};
    get_ground_atoms_from_ids(atom_ids, result);
    return result;
}

template<std::ranges::forward_range Iterable>
void PDDLFactories::get_objects_from_ids(const Iterable& object_ids, ObjectList& out_objects) const
{
    out_objects.clear();
    for (const auto& object_id : object_ids)
    {
        out_objects.push_back(get_object(object_id));
    }
}

template<std::ranges::forward_range Iterable>
ObjectList PDDLFactories::get_objects_from_ids(const Iterable& object_ids) const
{
    auto objects = ObjectList {};
    get_objects_from_ids(object_ids, objects);
    return objects;
}

}

#endif
