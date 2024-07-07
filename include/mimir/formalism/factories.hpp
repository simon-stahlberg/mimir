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

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/grounding_table.hpp"
#include "mimir/formalism/pddl.hpp"

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
        auto grounded_atom = get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
        auto grounded_literal = get_or_create_ground_literal(literal->is_negated(), grounded_atom);

        /* 3. Insert to groundings table */

        groundings.emplace(ObjectList(binding), GroundLiteral<P>(grounded_literal));

        /* 4. Return the resulting ground literal */

        return grounded_literal;
    }

public:
    PDDLFactories() : m_factories() {}

    // delete copy and allow move
    PDDLFactories(const PDDLFactories& other) = delete;
    PDDLFactories& operator=(const PDDLFactories& other) = delete;
    PDDLFactories(PDDLFactories&& other) = default;
    PDDLFactories& operator=(PDDLFactories&& other) = default;

    /// @brief Get or create requriements for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Requirements get_or_create_requirements(loki::RequirementEnumSet requirement_set)
    {
        return m_factories.get<RequirementsImpl>().get_or_create<RequirementsImpl>(std::move(requirement_set));
    }

    /// @brief Get or create a variable for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Variable get_or_create_variable(std::string name, size_t parameter_index)
    {
        return m_factories.get<VariableImpl>().get_or_create<VariableImpl>(std::move(name), std::move(parameter_index));
    }

    /// @brief Get or create a variable term for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Term get_or_create_term_variable(Variable variable) { return m_factories.get<TermImpl>().get_or_create<TermVariableImpl>(std::move(variable)); }

    /// @brief Get or create a object term for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Term get_or_create_term_object(Object object) { return m_factories.get<TermImpl>().get_or_create<TermObjectImpl>(std::move(object)); }

    /// @brief Get or create an object for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Object get_or_create_object(std::string name) { return m_factories.get<ObjectImpl>().get_or_create<ObjectImpl>(std::move(name)); }

    template<PredicateCategory P>
    Atom<P> get_or_create_atom(Predicate<P> predicate, TermList terms)
    {
        return m_factories.get<AtomImpl<P>>().template get_or_create<AtomImpl<P>>(std::move(predicate), std::move(terms));
    }

    template<PredicateCategory P>
    GroundAtom<P> get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
    {
        return m_factories.get<GroundAtomImpl<P>>().template get_or_create<GroundAtomImpl<P>>(std::move(predicate), std::move(objects));
    }

    template<PredicateCategory P>
    Literal<P> get_or_create_literal(bool is_negated, Atom<P> atom)
    {
        return m_factories.get<LiteralImpl<P>>().template get_or_create<LiteralImpl<P>>(is_negated, std::move(atom));
    }

    template<PredicateCategory P>
    GroundLiteral<P> get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom)
    {
        return m_factories.get<GroundLiteralImpl<P>>().template get_or_create<GroundLiteralImpl<P>>(is_negated, std::move(atom));
    }

    template<PredicateCategory P>
    Predicate<P> get_or_create_predicate(std::string name, VariableList parameters)
    {
        return m_factories.get<PredicateImpl<P>>().template get_or_create<PredicateImpl<P>>(name, std::move(parameters));
    }

    /// @brief Get or create a number function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_number(double number)
    {
        return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionNumberImpl>(number);
    }

    /// @brief Get or create a binary operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                         FunctionExpression left_function_expression,
                                                                         FunctionExpression right_function_expression)
    {
        return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                                             std::move(left_function_expression),
                                                                                                             std::move(right_function_expression));
    }

    /// @brief Get or create a multi operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator, FunctionExpressionList function_expressions_)
    {
        return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions_));
    }

    /// @brief Get or create a minus function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_minus(FunctionExpression function_expression)
    {
        return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionMinusImpl>(std::move(function_expression));
    }

    /// @brief Get or create a function function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionExpression get_or_create_function_expression_function(Function function)
    {
        return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionFunctionImpl>(std::move(function));
    }

    /// @brief Get or create a number function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_number(double number)
    {
        return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionNumberImpl>(number);
    }

    /// @brief Get or create a binary operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                      GroundFunctionExpression left_function_expression,
                                                                                      GroundFunctionExpression right_function_expression)
    {
        return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                                                         std::move(left_function_expression),
                                                                                                                         std::move(right_function_expression));
    }

    /// @brief Get or create a multi operator function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                     GroundFunctionExpressionList function_expressions_)
    {
        return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionMultiOperatorImpl>(multi_operator,
                                                                                                                        std::move(function_expressions_));
    }

    /// @brief Get or create a minus function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
    {
        return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionMinusImpl>(std::move(function_expression));
    }

    /// @brief Get or create a function function expression for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunctionExpression get_or_create_ground_function_expression_function(GroundFunction function)
    {
        return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionFunctionImpl>(std::move(function));
    }

    /// @brief Get or create a function for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Function get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
    {
        return m_factories.get<FunctionImpl>().get_or_create<FunctionImpl>(std::move(function_skeleton), std::move(terms));
    }

    /// @brief Get or create a function for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    GroundFunction get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects)
    {
        return m_factories.get<GroundFunctionImpl>().get_or_create<GroundFunctionImpl>(std::move(function_skeleton), std::move(objects));
    }

    /// @brief Get or create a function skeleton for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    FunctionSkeleton get_or_create_function_skeleton(std::string name, VariableList parameters)
    {
        return m_factories.get<FunctionSkeletonImpl>().get_or_create<FunctionSkeletonImpl>(std::move(name), std::move(parameters));
    }

    /// @brief Get or create a simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectSimple get_or_create_simple_effect(Literal<Fluent> effect)
    {
        return m_factories.get<EffectSimpleImpl>().get_or_create<EffectSimpleImpl>(std::move(effect));
    }

    /// @brief Get or create a conditional simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectConditional get_or_create_conditional_effect(LiteralList<Static> static_conditions,
                                                       LiteralList<Fluent> fluent_conditions,
                                                       LiteralList<Derived> derived_conditions,
                                                       Literal<Fluent> effect)
    {
        return m_factories.get<EffectConditionalImpl>().get_or_create<EffectConditionalImpl>(std::move(static_conditions),
                                                                                             std::move(fluent_conditions),
                                                                                             std::move(derived_conditions),
                                                                                             std::move(effect));
    }

    /// @brief Get or create a universal conditional simple effect for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    EffectUniversal get_or_create_universal_effect(VariableList parameters,
                                                   LiteralList<Static> static_conditions,
                                                   LiteralList<Fluent> fluent_conditions,
                                                   LiteralList<Derived> derived_conditions,
                                                   Literal<Fluent> effect)
    {
        return m_factories.get<EffectUniversalImpl>().get_or_create<EffectUniversalImpl>(std::move(parameters),
                                                                                         std::move(static_conditions),
                                                                                         std::move(fluent_conditions),
                                                                                         std::move(derived_conditions),
                                                                                         std::move(effect));
    }

    /// @brief Get or create an action for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Action get_or_create_action(std::string name,
                                size_t original_arity,
                                VariableList parameters,
                                LiteralList<Static> static_conditions,
                                LiteralList<Fluent> fluent_conditions,
                                LiteralList<Derived> derived_conditions,
                                EffectSimpleList simple_effects,
                                EffectConditionalList conditional_effects,
                                EffectUniversalList universal_effects,
                                FunctionExpression function_expression)
    {
        return m_factories.get<ActionImpl>().get_or_create<ActionImpl>(std::move(name),
                                                                       std::move(original_arity),
                                                                       std::move(parameters),
                                                                       std::move(static_conditions),
                                                                       std::move(fluent_conditions),
                                                                       std::move(derived_conditions),
                                                                       std::move(simple_effects),
                                                                       std::move(conditional_effects),
                                                                       std::move(universal_effects),
                                                                       std::move(function_expression));
    }

    /// @brief Get or create a derived predicate for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Axiom get_or_create_axiom(VariableList parameters,
                              Literal<Derived> literal,
                              LiteralList<Static> static_condition,
                              LiteralList<Fluent> fluent_condition,
                              LiteralList<Derived> derived_conditions)
    {
        return m_factories.get<AxiomImpl>().get_or_create<AxiomImpl>(std::move(parameters),
                                                                     std::move(literal),
                                                                     std::move(static_condition),
                                                                     std::move(fluent_condition),
                                                                     std::move(derived_conditions));
    }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    OptimizationMetric get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
    {
        return m_factories.get<OptimizationMetricImpl>().get_or_create<OptimizationMetricImpl>(std::move(metric), std::move(function_expression));
    }

    /// @brief Get or create an optimization metric for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    NumericFluent get_or_create_numeric_fluent(GroundFunction function, double number)
    {
        return m_factories.get<NumericFluentImpl>().get_or_create<NumericFluentImpl>(std::move(function), std::move(number));
    }

    /// @brief Get or create a domain for the given parameters.
    ///
    ///        This function allows us to can change the underlying representation and storage.
    Domain get_or_create_domain(std::string name,
                                Requirements requirements,
                                ObjectList constants,
                                PredicateList<Static> static_predicates,
                                PredicateList<Fluent> fluent_predicates,
                                PredicateList<Derived> derived_predicates,
                                FunctionSkeletonList functions,
                                ActionList actions,
                                AxiomList axioms)
    {
        return m_factories.get<DomainImpl>().get_or_create<DomainImpl>(std::move(name),
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
                                  PredicateList<Derived> derived_predicates,
                                  PredicateList<Derived> problem_and_domain_derived_predicates,
                                  GroundLiteralList<Static> static_initial_literals,
                                  GroundLiteralList<Fluent> fluent_initial_literals,
                                  NumericFluentList numeric_fluents,
                                  GroundLiteralList<Static> static_goal_condition,
                                  GroundLiteralList<Fluent> fluent_goal_condition,
                                  GroundLiteralList<Derived> derived_goal_condition,
                                  std::optional<OptimizationMetric> optimization_metric,
                                  AxiomList axioms)
    {
        return m_factories.get<ProblemImpl>().get_or_create<ProblemImpl>(std::move(domain),
                                                                         std::move(name),
                                                                         std::move(requirements),
                                                                         std::move(objects),
                                                                         std::move(derived_predicates),
                                                                         std::move(problem_and_domain_derived_predicates),
                                                                         std::move(static_initial_literals),
                                                                         std::move(fluent_initial_literals),
                                                                         std::move(numeric_fluents),
                                                                         std::move(static_goal_condition),
                                                                         std::move(fluent_goal_condition),
                                                                         std::move(derived_goal_condition),
                                                                         std::move(optimization_metric),
                                                                         std::move(axioms));
    }

    /* Accessors */

    // Factory
    template<typename T>
    const loki::PDDLFactory<T>& get_factory() const
    {
        return m_factories.get<T>();
    }

    // GroundAtom
    template<PredicateCategory P>
    GroundAtom<P> get_ground_atom(size_t atom_id) const
    {
        return m_factories.get<GroundAtomImpl<P>>().at(atom_id);
    }

    template<PredicateCategory P, std::ranges::forward_range Iterable>
    void get_ground_atoms_from_ids(const Iterable& atom_ids, GroundAtomList<P>& out_ground_atoms) const
    {
        out_ground_atoms.clear();

        for (const auto& atom_id : atom_ids)
        {
            out_ground_atoms.push_back(get_ground_atom<P>(atom_id));
        }
    }

    template<PredicateCategory P, std::ranges::forward_range Iterable>
    GroundAtomList<P> get_ground_atoms_from_ids(const Iterable& atom_ids) const
    {
        auto result = GroundAtomList<P> {};
        get_ground_atoms_from_ids(atom_ids, result);
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

    Object get_object(size_t object_id) const { return get_factory<ObjectImpl>().at(object_id); }

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

    GroundLiteral<Static> ground_literal(const Literal<Static> literal, const ObjectList& binding)
    {
        return ground_literal_generic(literal, binding, m_groundings_by_static_literal);
    }

    GroundLiteral<Fluent> ground_literal(const Literal<Fluent> literal, const ObjectList& binding)
    {
        return ground_literal_generic(literal, binding, m_groundings_by_fluent_literal);
    }

    GroundLiteral<Derived> ground_literal(const Literal<Derived> literal, const ObjectList& binding)
    {
        return ground_literal_generic(literal, binding, m_groundings_by_derived_literal);
    }

    template<PredicateCategory P>
    void ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                FlatBitsetBuilder<P>& ref_positive_bitset,
                                FlatBitsetBuilder<P>& ref_negative_bitset,
                                const auto& binding)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = ground_literal(literal, binding);

            if (grounded_literal->is_negated())
            {
                ref_negative_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
            else
            {
                ref_positive_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
        }
    }

    template<PredicateCategory P>
    void ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                FlatIndexListBuilder& ref_positive_indices,
                                FlatIndexListBuilder& ref_negative_indices,
                                const auto& binding)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = ground_literal(literal, binding);

            if (grounded_literal->is_negated())
            {
                ref_negative_indices.push_back(grounded_literal->get_atom()->get_identifier());
            }
            else
            {
                ref_positive_indices.push_back(grounded_literal->get_atom()->get_identifier());
            }
        }
    }
};

}

#endif
