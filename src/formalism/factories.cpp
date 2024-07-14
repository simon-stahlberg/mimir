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

#include "mimir/formalism/factories.hpp"

namespace mimir
{

PDDLFactories::PDDLFactories() : m_factories() {}

PDDLFactories::PDDLFactories(PDDLFactories&& other) = default;

PDDLFactories& PDDLFactories::operator=(PDDLFactories&& other) = default;

Requirements PDDLFactories::get_or_create_requirements(loki::RequirementEnumSet requirement_set)
{
    return m_factories.get<RequirementsImpl>().get_or_create<RequirementsImpl>(std::move(requirement_set));
}

Variable PDDLFactories::get_or_create_variable(std::string name, size_t parameter_index)
{
    return m_factories.get<VariableImpl>().get_or_create<VariableImpl>(std::move(name), std::move(parameter_index));
}

Term PDDLFactories::get_or_create_term_variable(Variable variable) { return m_factories.get<TermImpl>().get_or_create<TermVariableImpl>(std::move(variable)); }

Term PDDLFactories::get_or_create_term_object(Object object) { return m_factories.get<TermImpl>().get_or_create<TermObjectImpl>(std::move(object)); }

Object PDDLFactories::get_or_create_object(std::string name) { return m_factories.get<ObjectImpl>().get_or_create<ObjectImpl>(std::move(name)); }

template<PredicateCategory P>
Atom<P> PDDLFactories::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return m_factories.get<AtomImpl<P>>().template get_or_create<AtomImpl<P>>(std::move(predicate), std::move(terms));
}

template Atom<Static> PDDLFactories::get_or_create_atom(Predicate<Static> predicate, TermList terms);
template Atom<Fluent> PDDLFactories::get_or_create_atom(Predicate<Fluent> predicate, TermList terms);
template Atom<Derived> PDDLFactories::get_or_create_atom(Predicate<Derived> predicate, TermList terms);

template<PredicateCategory P>
GroundAtom<P> PDDLFactories::get_or_create_ground_atom(Predicate<P> predicate, ObjectList objects)
{
    return m_factories.get<GroundAtomImpl<P>>().template get_or_create<GroundAtomImpl<P>>(std::move(predicate), std::move(objects));
}

template GroundAtom<Static> PDDLFactories::get_or_create_ground_atom(Predicate<Static> predicate, ObjectList ObjectList);
template GroundAtom<Fluent> PDDLFactories::get_or_create_ground_atom(Predicate<Fluent> predicate, ObjectList ObjectList);
template GroundAtom<Derived> PDDLFactories::get_or_create_ground_atom(Predicate<Derived> predicate, ObjectList ObjectList);

template<PredicateCategory P>
Literal<P> PDDLFactories::get_or_create_literal(bool is_negated, Atom<P> atom)
{
    return m_factories.get<LiteralImpl<P>>().template get_or_create<LiteralImpl<P>>(is_negated, std::move(atom));
}

template Literal<Static> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Static> atom);
template Literal<Fluent> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Fluent> atom);
template Literal<Derived> PDDLFactories::get_or_create_literal(bool is_negated, Atom<Derived> atom);

template<PredicateCategory P>
GroundLiteral<P> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<P> atom)
{
    return m_factories.get<GroundLiteralImpl<P>>().template get_or_create<GroundLiteralImpl<P>>(is_negated, std::move(atom));
}

template GroundLiteral<Static> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Static> atom);
template GroundLiteral<Fluent> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Fluent> atom);
template GroundLiteral<Derived> PDDLFactories::get_or_create_ground_literal(bool is_negated, GroundAtom<Derived> atom);

template<PredicateCategory P>
Predicate<P> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters)
{
    return m_factories.get<PredicateImpl<P>>().template get_or_create<PredicateImpl<P>>(name, std::move(parameters));
}

template Predicate<Static> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Fluent> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);
template Predicate<Derived> PDDLFactories::get_or_create_predicate(std::string name, VariableList parameters);

FunctionExpression PDDLFactories::get_or_create_function_expression_number(double number)
{
    return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionNumberImpl>(number);
}

FunctionExpression PDDLFactories::get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                    FunctionExpression left_function_expression,
                                                                                    FunctionExpression right_function_expression)
{
    return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                                         std::move(left_function_expression),
                                                                                                         std::move(right_function_expression));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                   FunctionExpressionList function_expressions_)
{
    return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionMultiOperatorImpl>(multi_operator, std::move(function_expressions_));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_minus(FunctionExpression function_expression)
{
    return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionMinusImpl>(std::move(function_expression));
}

FunctionExpression PDDLFactories::get_or_create_function_expression_function(Function function)
{
    return m_factories.get<FunctionExpressionImpl>().get_or_create<FunctionExpressionFunctionImpl>(std::move(function));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_number(double number)
{
    return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionNumberImpl>(number);
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                 GroundFunctionExpression left_function_expression,
                                                                                                 GroundFunctionExpression right_function_expression)
{
    return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionBinaryOperatorImpl>(binary_operator,
                                                                                                                     std::move(left_function_expression),
                                                                                                                     std::move(right_function_expression));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                                GroundFunctionExpressionList function_expressions_)
{
    return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionMultiOperatorImpl>(multi_operator,
                                                                                                                    std::move(function_expressions_));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_minus(GroundFunctionExpression function_expression)
{
    return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionMinusImpl>(std::move(function_expression));
}

GroundFunctionExpression PDDLFactories::get_or_create_ground_function_expression_function(GroundFunction function)
{
    return m_factories.get<GroundFunctionExpressionImpl>().get_or_create<GroundFunctionExpressionFunctionImpl>(std::move(function));
}

Function PDDLFactories::get_or_create_function(FunctionSkeleton function_skeleton, TermList terms)
{
    return m_factories.get<FunctionImpl>().get_or_create<FunctionImpl>(std::move(function_skeleton), std::move(terms));
}

GroundFunction PDDLFactories::get_or_create_ground_function(FunctionSkeleton function_skeleton, ObjectList objects)
{
    return m_factories.get<GroundFunctionImpl>().get_or_create<GroundFunctionImpl>(std::move(function_skeleton), std::move(objects));
}

FunctionSkeleton PDDLFactories::get_or_create_function_skeleton(std::string name, VariableList parameters)
{
    return m_factories.get<FunctionSkeletonImpl>().get_or_create<FunctionSkeletonImpl>(std::move(name), std::move(parameters));
}

EffectSimple PDDLFactories::get_or_create_simple_effect(Literal<Fluent> effect)
{
    return m_factories.get<EffectSimpleImpl>().get_or_create<EffectSimpleImpl>(std::move(effect));
}

EffectConditional PDDLFactories::get_or_create_conditional_effect(LiteralList<Static> static_conditions,
                                                                  LiteralList<Fluent> fluent_conditions,
                                                                  LiteralList<Derived> derived_conditions,
                                                                  Literal<Fluent> effect)
{
    return m_factories.get<EffectConditionalImpl>().get_or_create<EffectConditionalImpl>(std::move(static_conditions),
                                                                                         std::move(fluent_conditions),
                                                                                         std::move(derived_conditions),
                                                                                         std::move(effect));
}

EffectUniversal PDDLFactories::get_or_create_universal_effect(VariableList parameters,
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

Action PDDLFactories::get_or_create_action(std::string name,
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

Axiom PDDLFactories::get_or_create_axiom(VariableList parameters,
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

OptimizationMetric PDDLFactories::get_or_create_optimization_metric(loki::OptimizationMetricEnum metric, GroundFunctionExpression function_expression)
{
    return m_factories.get<OptimizationMetricImpl>().get_or_create<OptimizationMetricImpl>(std::move(metric), std::move(function_expression));
}

NumericFluent PDDLFactories::get_or_create_numeric_fluent(GroundFunction function, double number)
{
    return m_factories.get<NumericFluentImpl>().get_or_create<NumericFluentImpl>(std::move(function), std::move(number));
}

Domain PDDLFactories::get_or_create_domain(std::optional<fs::path> filepath,
                                           std::string name,
                                           Requirements requirements,
                                           ObjectList constants,
                                           PredicateList<Static> static_predicates,
                                           PredicateList<Fluent> fluent_predicates,
                                           PredicateList<Derived> derived_predicates,
                                           FunctionSkeletonList functions,
                                           ActionList actions,
                                           AxiomList axioms)
{
    return m_factories.get<DomainImpl>().get_or_create<DomainImpl>(std::move(filepath),
                                                                   std::move(name),
                                                                   std::move(requirements),
                                                                   std::move(constants),
                                                                   std::move(static_predicates),
                                                                   std::move(fluent_predicates),
                                                                   std::move(derived_predicates),
                                                                   std::move(functions),
                                                                   std::move(actions),
                                                                   std::move(axioms));
}

Problem PDDLFactories::get_or_create_problem(std::optional<fs::path> filepath,
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
                                             AxiomList axioms)
{
    return m_factories.get<ProblemImpl>().get_or_create<ProblemImpl>(std::move(filepath),
                                                                     std::move(domain),
                                                                     std::move(name),
                                                                     std::move(requirements),
                                                                     std::move(objects),
                                                                     std::move(derived_predicates),
                                                                     std::move(static_initial_literals),
                                                                     std::move(fluent_initial_literals),
                                                                     std::move(numeric_fluents),
                                                                     std::move(static_goal_condition),
                                                                     std::move(fluent_goal_condition),
                                                                     std::move(derived_goal_condition),
                                                                     std::move(optimization_metric),
                                                                     std::move(axioms));
}

// Factory
template<typename T>
const loki::PDDLFactory<T>& PDDLFactories::get_factory() const
{
    return m_factories.get<T>();
}

template const loki::PDDLFactory<GroundAtomImpl<Static>>& PDDLFactories::get_factory<GroundAtomImpl<Static>>() const;
template const loki::PDDLFactory<GroundAtomImpl<Fluent>>& PDDLFactories::get_factory<GroundAtomImpl<Fluent>>() const;
template const loki::PDDLFactory<GroundAtomImpl<Derived>>& PDDLFactories::get_factory<GroundAtomImpl<Derived>>() const;

// GroundAtom
template<PredicateCategory P>
GroundAtom<P> PDDLFactories::get_ground_atom(size_t atom_id) const
{
    return m_factories.get<GroundAtomImpl<P>>().at(atom_id);
}

template GroundAtom<Static> PDDLFactories::get_ground_atom<Static>(size_t atom_id) const;
template GroundAtom<Fluent> PDDLFactories::get_ground_atom<Fluent>(size_t atom_id) const;
template GroundAtom<Derived> PDDLFactories::get_ground_atom<Derived>(size_t atom_id) const;

Object PDDLFactories::get_object(size_t object_id) const { return get_factory<ObjectImpl>().at(object_id); }

/* Grounding */

void PDDLFactories::ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
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

template<PredicateCategory P>
GroundLiteral<P>
PDDLFactories::ground_literal_generic(const Literal<P> literal, const ObjectList& binding, std::vector<GroundingTable<GroundLiteral<P>>>& grounding_table)
{
    /* 1. Check if grounding is cached */
    const auto literal_id = literal->get_identifier();
    if (literal_id >= grounding_table.size())
    {
        grounding_table.resize(literal_id + 1);
    }

    auto& groundings = grounding_table[literal_id];

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

template GroundLiteral<Static> PDDLFactories::ground_literal_generic(const Literal<Static> literal,
                                                                     const ObjectList& binding,
                                                                     std::vector<GroundingTable<GroundLiteral<Static>>>& grounding_table);
template GroundLiteral<Fluent> PDDLFactories::ground_literal_generic(const Literal<Fluent> literal,
                                                                     const ObjectList& binding,
                                                                     std::vector<GroundingTable<GroundLiteral<Fluent>>>& grounding_table);
template GroundLiteral<Derived> PDDLFactories::ground_literal_generic(const Literal<Derived> literal,
                                                                      const ObjectList& binding,
                                                                      std::vector<GroundingTable<GroundLiteral<Derived>>>& grounding_table);

GroundLiteral<Static> PDDLFactories::ground_literal(const Literal<Static> literal, const ObjectList& binding)
{
    return ground_literal_generic(literal, binding, m_groundings_by_static_literal);
}

GroundLiteral<Fluent> PDDLFactories::ground_literal(const Literal<Fluent> literal, const ObjectList& binding)
{
    return ground_literal_generic(literal, binding, m_groundings_by_fluent_literal);
}

GroundLiteral<Derived> PDDLFactories::ground_literal(const Literal<Derived> literal, const ObjectList& binding)
{
    return ground_literal_generic(literal, binding, m_groundings_by_derived_literal);
}

template<PredicateCategory P>
void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                           FlatBitsetBuilder<P>& ref_positive_bitset,
                                           FlatBitsetBuilder<P>& ref_negative_bitset,
                                           const ObjectList& binding)
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

template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Static>>& literals,
                                                    FlatBitsetBuilder<Static>& ref_positive_bitset,
                                                    FlatBitsetBuilder<Static>& ref_negative_bitset,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Fluent>>& literals,
                                                    FlatBitsetBuilder<Fluent>& ref_positive_bitset,
                                                    FlatBitsetBuilder<Fluent>& ref_negative_bitset,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_bitset(const std::vector<Literal<Derived>>& literals,
                                                    FlatBitsetBuilder<Derived>& ref_positive_bitset,
                                                    FlatBitsetBuilder<Derived>& ref_negative_bitset,
                                                    const ObjectList& binding);

template<PredicateCategory P>
void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                           FlatIndexListBuilder& ref_positive_indices,
                                           FlatIndexListBuilder& ref_negative_indices,
                                           const ObjectList& binding)
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

template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Static>>& literals,
                                                    FlatIndexListBuilder& ref_positive_indices,
                                                    FlatIndexListBuilder& ref_negative_indices,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Fluent>>& literals,
                                                    FlatIndexListBuilder& ref_positive_indices,
                                                    FlatIndexListBuilder& ref_negative_indices,
                                                    const ObjectList& binding);
template void PDDLFactories::ground_and_fill_vector(const std::vector<Literal<Derived>>& literals,
                                                    FlatIndexListBuilder& ref_positive_indices,
                                                    FlatIndexListBuilder& ref_negative_indices,
                                                    const ObjectList& binding);
}
