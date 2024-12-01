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
#include "mimir/formalism/equal_to.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/formalism/hash.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>
#include <ranges>

namespace mimir
{

template<typename T>
using SegmentedPDDLRepository = loki::SegmentedRepository<T, UniquePDDLHasher<const T*>, UniquePDDLEqualTo<const T*>>;

using RequirementsRepository = SegmentedPDDLRepository<RequirementsImpl>;
using VariableRepository = SegmentedPDDLRepository<VariableImpl>;
using TermRepository = SegmentedPDDLRepository<TermImpl>;
using ObjectRepository = SegmentedPDDLRepository<ObjectImpl>;
template<PredicateTag P>
using AtomRepository = SegmentedPDDLRepository<AtomImpl<P>>;
template<PredicateTag P>
using GroundAtomRepository = SegmentedPDDLRepository<GroundAtomImpl<P>>;
template<PredicateTag P>
using LiteralRepository = SegmentedPDDLRepository<LiteralImpl<P>>;
template<PredicateTag P>
using GroundLiteralRepository = SegmentedPDDLRepository<GroundLiteralImpl<P>>;
template<PredicateTag P>
using PredicateRepository = SegmentedPDDLRepository<PredicateImpl<P>>;
using FunctionExpressionNumberRepository = SegmentedPDDLRepository<FunctionExpressionNumberImpl>;
using FunctionExpressionBinaryOperatorRepository = SegmentedPDDLRepository<FunctionExpressionBinaryOperatorImpl>;
using FunctionExpressionMultiOperatorRepository = SegmentedPDDLRepository<FunctionExpressionMultiOperatorImpl>;
using FunctionExpressionMinusRepository = SegmentedPDDLRepository<FunctionExpressionMinusImpl>;
using FunctionExpressionFunctionRepository = SegmentedPDDLRepository<FunctionExpressionFunctionImpl>;
using FunctionExpressionRepository = SegmentedPDDLRepository<FunctionExpressionImpl>;
using GroundFunctionExpressionNumberRepository = SegmentedPDDLRepository<GroundFunctionExpressionNumberImpl>;
using GroundFunctionExpressionBinaryOperatorRepository = SegmentedPDDLRepository<GroundFunctionExpressionBinaryOperatorImpl>;
using GroundFunctionExpressionMultiOperatorRepository = SegmentedPDDLRepository<GroundFunctionExpressionMultiOperatorImpl>;
using GroundFunctionExpressionMinusRepository = SegmentedPDDLRepository<GroundFunctionExpressionMinusImpl>;
using GroundFunctionExpressionFunctionRepository = SegmentedPDDLRepository<GroundFunctionExpressionFunctionImpl>;
using GroundFunctionExpressionRepository = SegmentedPDDLRepository<GroundFunctionExpressionImpl>;
using FunctionRepository = SegmentedPDDLRepository<FunctionImpl>;
using GroundFunctionRepository = SegmentedPDDLRepository<GroundFunctionImpl>;
using FunctionSkeletonRepository = SegmentedPDDLRepository<FunctionSkeletonImpl>;
using EffectStripsRepository = SegmentedPDDLRepository<EffectStripsImpl>;
using EffectUniversalRepository = SegmentedPDDLRepository<EffectConditionalImpl>;
using ActionRepository = SegmentedPDDLRepository<ActionImpl>;
using AxiomRepository = SegmentedPDDLRepository<AxiomImpl>;
using OptimizationMetricRepository = SegmentedPDDLRepository<OptimizationMetricImpl>;
using NumericFluentRepository = SegmentedPDDLRepository<NumericFluentImpl>;
using DomainRepository = SegmentedPDDLRepository<DomainImpl>;
using ProblemRepository = SegmentedPDDLRepository<ProblemImpl>;

using PDDLTypeToRepository =
    boost::hana::map<boost::hana::pair<boost::hana::type<RequirementsImpl>, RequirementsRepository>,
                     boost::hana::pair<boost::hana::type<VariableImpl>, VariableRepository>,
                     boost::hana::pair<boost::hana::type<TermImpl>, TermRepository>,
                     boost::hana::pair<boost::hana::type<ObjectImpl>, ObjectRepository>,
                     boost::hana::pair<boost::hana::type<AtomImpl<Static>>, AtomRepository<Static>>,
                     boost::hana::pair<boost::hana::type<AtomImpl<Fluent>>, AtomRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<AtomImpl<Derived>>, AtomRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<GroundAtomImpl<Static>>, GroundAtomRepository<Static>>,
                     boost::hana::pair<boost::hana::type<GroundAtomImpl<Fluent>>, GroundAtomRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<GroundAtomImpl<Derived>>, GroundAtomRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<LiteralImpl<Static>>, LiteralRepository<Static>>,
                     boost::hana::pair<boost::hana::type<LiteralImpl<Fluent>>, LiteralRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<LiteralImpl<Derived>>, LiteralRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<GroundLiteralImpl<Static>>, GroundLiteralRepository<Static>>,
                     boost::hana::pair<boost::hana::type<GroundLiteralImpl<Fluent>>, GroundLiteralRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<GroundLiteralImpl<Derived>>, GroundLiteralRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<PredicateImpl<Static>>, PredicateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<PredicateImpl<Fluent>>, PredicateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<PredicateImpl<Derived>>, PredicateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionNumberImpl>, FunctionExpressionNumberRepository>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionBinaryOperatorImpl>, FunctionExpressionBinaryOperatorRepository>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionMultiOperatorImpl>, FunctionExpressionMultiOperatorRepository>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionMinusImpl>, FunctionExpressionMinusRepository>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionFunctionImpl>, FunctionExpressionFunctionRepository>,
                     boost::hana::pair<boost::hana::type<FunctionExpressionImpl>, FunctionExpressionRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionNumberImpl>, GroundFunctionExpressionNumberRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionBinaryOperatorImpl>, GroundFunctionExpressionBinaryOperatorRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionMultiOperatorImpl>, GroundFunctionExpressionMultiOperatorRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionMinusImpl>, GroundFunctionExpressionMinusRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionFunctionImpl>, GroundFunctionExpressionFunctionRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionExpressionImpl>, GroundFunctionExpressionRepository>,
                     boost::hana::pair<boost::hana::type<FunctionImpl>, FunctionRepository>,
                     boost::hana::pair<boost::hana::type<GroundFunctionImpl>, GroundFunctionRepository>,
                     boost::hana::pair<boost::hana::type<FunctionSkeletonImpl>, FunctionSkeletonRepository>,
                     boost::hana::pair<boost::hana::type<EffectStripsImpl>, EffectStripsRepository>,
                     boost::hana::pair<boost::hana::type<EffectConditionalImpl>, EffectUniversalRepository>,
                     boost::hana::pair<boost::hana::type<ActionImpl>, ActionRepository>,
                     boost::hana::pair<boost::hana::type<AxiomImpl>, AxiomRepository>,
                     boost::hana::pair<boost::hana::type<OptimizationMetricImpl>, OptimizationMetricRepository>,
                     boost::hana::pair<boost::hana::type<NumericFluentImpl>, NumericFluentRepository>,
                     boost::hana::pair<boost::hana::type<DomainImpl>, DomainRepository>,
                     boost::hana::pair<boost::hana::type<ProblemImpl>, ProblemRepository>>;

extern PDDLTypeToRepository create_default_pddl_type_to_repository();

/// @brief Collection of factories for the unique creation of PDDL objects.
class PDDLRepositories
{
private:
    PDDLTypeToRepository m_repositories;

    using GroundedTypeToGroundingTableList =
        boost::hana::map<boost::hana::pair<boost::hana::type<GroundLiteral<Static>>, GroundingTableList<GroundLiteral<Static>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<Fluent>>, GroundingTableList<GroundLiteral<Fluent>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<Derived>>, GroundingTableList<GroundLiteral<Derived>>>>;

    GroundedTypeToGroundingTableList m_grounding_tables;

public:
    PDDLRepositories();

    // delete copy and allow move
    PDDLRepositories(const PDDLRepositories& other) = delete;
    PDDLRepositories& operator=(const PDDLRepositories& other) = delete;
    PDDLRepositories(PDDLRepositories&& other);
    PDDLRepositories& operator=(PDDLRepositories&& other);

    /// @brief Get or create requriements for the given parameters.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set);

    /// @brief Get or create a variable for the given parameters.
    Variable get_or_create_variable(std::string name, size_t parameter_index);

    /// @brief Get or create a variable term for the given parameters.
    Term get_or_create_term(Variable variable);
    Term get_or_create_term(Object object);

    /// @brief Get or create an object for the given parameters.
    Object get_or_create_object(std::string name);

    template<PredicateTag P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms);

    template<PredicateTag P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects);

    template<PredicateTag P>
    Literal<P> get_or_create_literal(bool is_negated, Atom<P> atom);

    template<PredicateTag P>
    GroundLiteral<P> get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom);

    template<PredicateTag P>
    Predicate<P> get_or_create_predicate(std::string name, VariableList parameters);

    /// @brief Get or create a number function expression for the given parameters.
    FunctionExpressionNumber get_or_create_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    FunctionExpressionBinaryOperator get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                       FunctionExpression left_function_expression,
                                                                                       FunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    FunctionExpressionMultiOperator get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     FunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    FunctionExpressionMinus get_or_create_function_expression_minus(FunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    FunctionExpressionFunction get_or_create_function_expression_function(Function function);

    /// @brief Get or create a function expression for the given parameters.
    FunctionExpression get_or_create_function_expression(FunctionExpressionNumber fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMultiOperator fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionMinus fexpr);
    FunctionExpression get_or_create_function_expression(FunctionExpressionFunction fexpr);

    /// @brief Get or create a number function expression for the given parameters.
    GroundFunctionExpressionNumber get_or_create_ground_function_expression_number(double number);

    /// @brief Get or create a binary operator function expression for the given parameters.
    GroundFunctionExpressionBinaryOperator get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                    GroundFunctionExpression left_function_expression,
                                                                                                    GroundFunctionExpression right_function_expression);

    /// @brief Get or create a multi operator function expression for the given parameters.
    GroundFunctionExpressionMultiOperator get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                                  GroundFunctionExpressionList function_expressions_);

    /// @brief Get or create a minus function expression for the given parameters.
    GroundFunctionExpressionMinus get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression);

    /// @brief Get or create a function function expression for the given parameters.
    GroundFunctionExpressionFunction get_or_create_ground_function_expression_function(GroundFunction function);

    /// @brief Get or create a function expression for the given parameters.
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionNumber fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionBinaryOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMultiOperator fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionMinus fexpr);
    GroundFunctionExpression get_or_create_ground_function_expression(GroundFunctionExpressionFunction fexpr);

    /// @brief Get or create a function for the given parameters.
    Function get_or_create_function(FunctionSkeleton function_skeleton, TermList terms);

    /// @brief Get or create a function for the given parameters.
    GroundFunction get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects);

    /// @brief Get or create a function skeleton for the given parameters.
    FunctionSkeleton get_or_create_function_skeleton(std::string name, VariableList parameters);

    /// @brief Get or create a simple effect for the given parameters.
    EffectStrips get_or_create_strips_effect(LiteralList<Fluent> effects, FunctionExpression function_expression);

    /// @brief Get or create a universal conditional simple effect for the given parameters.
    EffectConditional get_or_create_conditional_effect(VariableList parameters,
                                                       LiteralList<Static> static_conditions,
                                                       LiteralList<Fluent> fluent_conditions,
                                                       LiteralList<Derived> derived_conditions,
                                                       LiteralList<Fluent> effects,
                                                       FunctionExpression function_expression);

    /// @brief Get or create an action for the given parameters.
    Action get_or_create_action(std::string name,
                                size_t original_arity,
                                VariableList parameters,
                                LiteralList<Static> static_conditions,
                                LiteralList<Fluent> fluent_conditions,
                                LiteralList<Derived> derived_conditions,
                                EffectStrips strips_effect,
                                EffectConditionalList conditional_effects);

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
    const PDDLTypeToRepository& get_pddl_type_to_factory() const;

    // GroundAtom
    template<PredicateTag P>
    GroundAtom<P> get_ground_atom(size_t atom_index) const;

    template<PredicateTag P, std::ranges::forward_range Iterable>
    void get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const;

    template<PredicateTag P, std::ranges::forward_range Iterable>
    GroundAtomList<P> get_ground_atoms_from_indices(const Iterable& atom_indices) const;

    template<PredicateTag P>
    void get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const;

    template<PredicateTag P>
    auto get_ground_atoms() const;

    // Object
    Object get_object(size_t object_index) const;

    template<std::ranges::forward_range Iterable>
    void get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const;

    template<std::ranges::forward_range Iterable>
    ObjectList get_objects_from_indices(const Iterable& object_indices) const;

    // Action
    Action get_action(size_t action_index) const;

    // Axiom
    Axiom get_axiom(size_t axiom_index) const;

    /* Grounding */

    void ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms);

    template<PredicateTag P>
    GroundLiteral<P> ground_literal(const Literal<P> literal, const ObjectList& binding);

    template<PredicateTag P>
    void ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                FlatBitset& ref_positive_bitset,
                                FlatBitset& ref_negative_bitset,
                                const ObjectList& binding);

    template<PredicateTag P>
    void ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                FlatIndexList& ref_positive_indices,
                                FlatIndexList& ref_negative_indices,
                                const ObjectList& binding);
};

/**
 * Implementations
 */

template<PredicateTag P, std::ranges::forward_range Iterable>
void PDDLRepositories::get_ground_atoms_from_indices(const Iterable& atom_indices, GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();

    for (const auto& atom_index : atom_indices)
    {
        out_ground_atoms.push_back(get_ground_atom<P>(atom_index));
    }
}

template<PredicateTag P, std::ranges::forward_range Iterable>
GroundAtomList<P> PDDLRepositories::get_ground_atoms_from_indices(const Iterable& atom_indices) const
{
    auto result = GroundAtomList<P> {};
    get_ground_atoms_from_indices(atom_indices, result);
    return result;
}

template<PredicateTag P>
void PDDLRepositories::get_ground_atoms(GroundAtomList<P>& out_ground_atoms) const
{
    out_ground_atoms.clear();
    for (const auto& atom : boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {}))
    {
        out_ground_atoms.push_back(atom);
    }
}

template<PredicateTag P>
auto PDDLRepositories::get_ground_atoms() const
{
    const auto& factory = boost::hana::at_key(m_repositories, boost::hana::type<GroundAtomImpl<P>> {});
    return std::ranges::subrange(factory.begin(), factory.end());
}

template<std::ranges::forward_range Iterable>
void PDDLRepositories::get_objects_from_indices(const Iterable& object_indices, ObjectList& out_objects) const
{
    out_objects.clear();
    for (const auto& object_index : object_indices)
    {
        out_objects.push_back(get_object(object_index));
    }
}

template<std::ranges::forward_range Iterable>
ObjectList PDDLRepositories::get_objects_from_indices(const Iterable& object_indices) const
{
    auto objects = ObjectList {};
    get_objects_from_indices(object_indices, objects);
    return objects;
}

}

#endif
