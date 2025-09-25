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

#include "mimir/formalism/problem.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/consistency_graph.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_function_value.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/utils.hpp"

#include <cassert>
#include <iostream>

using namespace std;

namespace mimir::formalism
{
ProblemImpl::ProblemImpl(Index index,
                         Repositories repositories,
                         std::optional<fs::path> filepath,
                         Domain domain,
                         std::string name,
                         Requirements requirements,
                         ObjectList objects,
                         ObjectList problem_and_domain_objects,
                         PredicateList<DerivedTag> derived_predicates,
                         PredicateList<DerivedTag> problem_and_domain_derived_predicates,
                         GroundLiteralLists<StaticTag, FluentTag> initial_literals,
                         GroundFunctionValueLists<StaticTag, FluentTag> initial_function_values,
                         std::optional<GroundFunctionValue<AuxiliaryTag>> auxiliary_function_value,
                         GroundLiteralLists<StaticTag, FluentTag, DerivedTag> goal_condition,
                         GroundNumericConstraintList numeric_goal_condition,
                         std::optional<OptimizationMetric> optimization_metric,
                         AxiomList axioms,
                         AxiomList problem_and_domain_axioms) :
    m_index(index),
    m_repositories(std::move(repositories)),
    m_filepath(std::move(filepath)),
    m_domain(std::move(domain)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_objects(std::move(objects)),
    m_problem_and_domain_objects(std::move(problem_and_domain_objects)),
    m_derived_predicates(std::move(derived_predicates)),
    m_problem_and_domain_derived_predicates(std::move(problem_and_domain_derived_predicates)),
    m_initial_literals(std::move(initial_literals)),
    m_initial_function_values(std::move(initial_function_values)),
    m_auxiliary_function_value(auxiliary_function_value),
    m_goal_condition(std::move(goal_condition)),
    m_numeric_goal_condition(std::move(numeric_goal_condition)),
    m_optimization_metric(std::move(optimization_metric)),
    m_axioms(std::move(axioms)),
    m_problem_and_domain_axioms(std::move(problem_and_domain_axioms)),
    m_details(),
    m_flat_index_list_map(),
    m_flat_index_lists(),
    m_flat_double_list_map(),
    m_flat_double_lists(),
    m_index_tree_table(),
    m_double_leaf_table(),
    m_bitset_pool(),
    m_index_list_pool(),
    m_double_list_pool()
{
    assert(is_all_unique(get_objects()));
    assert(is_all_unique(get_derived_predicates()));
    assert(is_all_unique(get_initial_literals<StaticTag>()));
    assert(is_all_unique(get_initial_literals<FluentTag>()));
    assert(is_all_unique(get_initial_function_values<StaticTag>()));
    assert(is_all_unique(get_initial_function_values<FluentTag>()));
    assert(is_all_unique(get_goal_condition<StaticTag>()));
    assert(is_all_unique(get_goal_condition<FluentTag>()));
    assert(is_all_unique(get_goal_condition<DerivedTag>()));
    assert(is_all_unique(get_numeric_goal_condition()));
    assert(is_all_unique(get_axioms()));
    assert(std::is_sorted(get_objects().begin(), get_objects().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_derived_predicates().begin(),
                          get_derived_predicates().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_initial_literals<StaticTag>().begin(),
                          get_initial_literals<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_initial_literals<FluentTag>().begin(),
                          get_initial_literals<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_initial_function_values<StaticTag>().begin(),
                          get_initial_function_values<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_initial_function_values<FluentTag>().begin(),
                          get_initial_function_values<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_goal_condition<StaticTag>().begin(),
                          get_goal_condition<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_goal_condition<FluentTag>().begin(),
                          get_goal_condition<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_goal_condition<DerivedTag>().begin(),
                          get_goal_condition<DerivedTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_numeric_goal_condition().begin(),
                          get_numeric_goal_condition().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_axioms().begin(), get_axioms().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));

    /**
     * Error checking
     */

    for (const auto& literal : get_initial_literals<FluentTag>())
    {
        if (!literal->get_polarity())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& literal : get_initial_literals<StaticTag>())
    {
        if (!literal->get_polarity())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    for (const auto& axiom : get_problem_and_domain_axioms())
    {
        if (!axiom->get_literal()->get_polarity())
        {
            throw std::runtime_error("Negative literals in axiom heads is not supported.");
        }
    }

    /**
     * Details
     */

    m_details = problem::Details(*this);
}

Problem ProblemImpl::create(const fs::path& domain_filepath, const fs::path& problem_filepath, const loki::Options& options)
{
    return Parser(domain_filepath, options).parse_problem(problem_filepath, options);
}

Index ProblemImpl::get_index() const { return m_index; }

const Repositories& ProblemImpl::get_repositories() const { return m_repositories; }

const std::optional<fs::path>& ProblemImpl::get_filepath() const { return m_filepath; }

const Domain& ProblemImpl::get_domain() const { return m_domain; }

const std::string& ProblemImpl::get_name() const { return m_name; }

Requirements ProblemImpl::get_requirements() const { return m_requirements; }

const ObjectList& ProblemImpl::get_objects() const { return m_objects; }

const ObjectList& ProblemImpl::get_problem_and_domain_objects() const { return m_problem_and_domain_objects; }

const PredicateList<DerivedTag>& ProblemImpl::get_derived_predicates() const { return m_derived_predicates; }

const PredicateList<DerivedTag>& ProblemImpl::get_problem_and_domain_derived_predicates() const { return m_problem_and_domain_derived_predicates; }

template<IsStaticOrFluentTag P>
const GroundLiteralList<P>& ProblemImpl::get_initial_literals() const
{
    return boost::hana::at_key(m_initial_literals, boost::hana::type<P> {});
}

template const GroundLiteralList<StaticTag>& ProblemImpl::get_initial_literals() const;
template const GroundLiteralList<FluentTag>& ProblemImpl::get_initial_literals() const;

const GroundLiteralLists<StaticTag, FluentTag>& ProblemImpl::get_hana_initial_literals() const { return m_initial_literals; }

template<IsStaticOrFluentTag F>
const GroundFunctionValueList<F>& ProblemImpl::get_initial_function_values() const
{
    return boost::hana::at_key(m_initial_function_values, boost::hana::type<F> {});
}

const GroundFunctionValueLists<StaticTag, FluentTag>& ProblemImpl::get_hana_initial_function_values() const { return m_initial_function_values; }

template const GroundFunctionValueList<StaticTag>& ProblemImpl::get_initial_function_values() const;
template const GroundFunctionValueList<FluentTag>& ProblemImpl::get_initial_function_values() const;

const std::optional<GroundFunctionValue<AuxiliaryTag>>& ProblemImpl::get_auxiliary_function_value() const { return m_auxiliary_function_value; }

template<IsStaticOrFluentOrDerivedTag P>
const GroundLiteralList<P>& ProblemImpl::get_goal_condition() const
{
    return boost::hana::at_key(m_goal_condition, boost::hana::type<P> {});
}

template const GroundLiteralList<StaticTag>& ProblemImpl::get_goal_condition<StaticTag>() const;
template const GroundLiteralList<FluentTag>& ProblemImpl::get_goal_condition<FluentTag>() const;
template const GroundLiteralList<DerivedTag>& ProblemImpl::get_goal_condition<DerivedTag>() const;

const GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_goal_condition() const { return m_goal_condition; }

const GroundNumericConstraintList& ProblemImpl::get_numeric_goal_condition() const { return m_numeric_goal_condition; }

const std::optional<OptimizationMetric>& ProblemImpl::get_optimization_metric() const { return m_optimization_metric; }

const AxiomList& ProblemImpl::get_axioms() const { return m_axioms; }

const AxiomList& ProblemImpl::get_problem_and_domain_axioms() const { return m_problem_and_domain_axioms; }

size_t ProblemImpl::get_estimated_memory_usage_in_bytes() const
{
    return m_flat_index_list_map.get_estimated_memory_usage_in_bytes() + m_flat_double_list_map.get_estimated_memory_usage_in_bytes();
}

/**
 * Additional members
 */

valla::IndexedHashSet<valla::Slot<Index>, Index>& ProblemImpl::get_index_tree_table() { return m_index_tree_table; }
const valla::IndexedHashSet<valla::Slot<Index>, Index>& ProblemImpl::get_index_tree_table() const { return m_index_tree_table; }

valla::IndexedHashSet<double, Index>& ProblemImpl::get_double_leaf_table() { return m_double_leaf_table; }
const valla::IndexedHashSet<double, Index>& ProblemImpl::get_double_leaf_table() const { return m_double_leaf_table; }

std::pair<const FlatIndexList*, Index> ProblemImpl::get_or_create_index_list(const FlatIndexList& list)
{
    auto result = m_flat_index_list_map.emplace(list, m_flat_index_list_map.size());
    const auto pointer = result.first->first.get();
    const auto index = result.first->second;

    if (result.second)
    {
        m_flat_index_lists.push_back(pointer);
    }

    return { pointer, index };
}
const FlatIndexList* ProblemImpl::get_index_list(size_t pos) const
{
    assert(pos < m_flat_index_lists.size());
    return m_flat_index_lists[pos];
}
std::pair<const FlatDoubleList*, Index> ProblemImpl::get_or_create_double_list(const FlatDoubleList& list)
{
    auto result = m_flat_double_list_map.emplace(list, m_flat_double_list_map.size());
    const auto pointer = result.first->first.get();
    const auto index = result.first->second;

    if (result.second)
    {
        m_flat_double_lists.push_back(pointer);
    }

    return { pointer, index };
}
const FlatDoubleList* ProblemImpl::get_double_list(size_t pos) const
{
    assert(pos < m_flat_double_lists.size());
    return m_flat_double_lists[pos];
}

SharedObjectPool<FlatBitset>& ProblemImpl::get_bitset_pool() { return m_bitset_pool; }
SharedObjectPool<FlatIndexList>& ProblemImpl::get_index_list_pool() { return m_index_list_pool; }
SharedObjectPool<FlatDoubleList>& ProblemImpl::get_double_list_pool() { return m_double_list_pool; }

/* Objects */
const Object ProblemImpl::get_object(const std::string& name) const { return get_name_to_object().at(name); }
const Object ProblemImpl::get_problem_or_domain_object(const std::string& name) const { return get_name_to_problem_or_domain_object().at(name); }
const ToObjectMap<std::string> ProblemImpl::get_name_to_object() const { return m_details.objects.name_to_object; }
const ToObjectMap<std::string> ProblemImpl::get_name_to_problem_or_domain_object() const { return m_details.objects.name_to_problem_or_domain_object; }

/* Predicates */

const Predicate<DerivedTag>& ProblemImpl::get_derived_predicate(const std::string& name) const { return get_name_to_derived_predicate().at(name); }
const Predicate<DerivedTag>& ProblemImpl::get_problem_or_domain_derived_predicate(const std::string& name) const
{
    return get_name_to_problem_or_domain_derived_predicate().at(name);
}
const ToPredicateMap<std::string, DerivedTag>& ProblemImpl::get_name_to_derived_predicate() const { return m_details.predicates.name_to_derived_predicate; }
const ToPredicateMap<std::string, DerivedTag>& ProblemImpl::get_name_to_problem_or_domain_derived_predicate() const
{
    return m_details.predicates.name_to_problem_or_domain_derived_predicate;
}

/* Initial state */

const GroundAtomList<StaticTag>& ProblemImpl::get_static_initial_atoms() const { return m_details.initial.positive_static_initial_atoms; }

const FlatBitset& ProblemImpl::get_positive_static_initial_atoms_bitset() const { return m_details.initial.positive_static_initial_atoms_bitset; }

const FlatIndexList& ProblemImpl::get_positive_static_initial_atoms_indices() const
{
    assert(std::is_sorted(m_details.initial.positive_static_initial_atoms_indices.uncompressed_begin(),
                          m_details.initial.positive_static_initial_atoms_indices.uncompressed_end()));
    return m_details.initial.positive_static_initial_atoms_indices;
}

const PredicateAssignmentSets<StaticTag>& ProblemImpl::get_positive_static_initial_predicate_assignment_sets() const
{
    return m_details.initial.positive_static_initial_predicate_assignment_sets;
}

const FunctionSkeletonAssignmentSets<StaticTag>& ProblemImpl::get_static_initial_function_skeleton_assignment_sets() const
{
    return m_details.initial.static_initial_function_skeleton_assignment_sets;
}

const GroundAtomList<FluentTag>& ProblemImpl::get_fluent_initial_atoms() const { return m_details.initial.positive_fluent_initial_atoms; }

template<IsStaticOrFluentTag F>
const FlatDoubleList& ProblemImpl::get_initial_function_to_value() const
{
    return boost::hana::at_key(m_details.initial.initial_function_to_value, boost::hana::type<F> {});
}

template const FlatDoubleList& ProblemImpl::get_initial_function_to_value<StaticTag>() const;
template const FlatDoubleList& ProblemImpl::get_initial_function_to_value<FluentTag>() const;

const FlatDoubleLists<StaticTag, FluentTag>& ProblemImpl::get_hana_initial_function_to_value() const { return m_details.initial.initial_function_to_value; }

template<IsStaticOrFluentTag F>
ContinuousCost ProblemImpl::get_initial_function_value(GroundFunction<F> function) const
{
    const auto& function_to_value = get_initial_function_to_value<F>();
    if (function->get_index() >= function_to_value.size())
    {
        return UNDEFINED_CONTINUOUS_COST;
    }
    return function_to_value[function->get_index()];
}

template ContinuousCost ProblemImpl::get_initial_function_value(GroundFunction<StaticTag> function) const;
template ContinuousCost ProblemImpl::get_initial_function_value(GroundFunction<FluentTag> function) const;

/* Goal */
bool ProblemImpl::static_goal_holds() const { return m_details.goal.m_static_goal_holds; }

template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
const GroundAtomList<P>& ProblemImpl::get_goal_atoms() const
{
    return boost::hana::at_key(boost::hana::at_key(m_details.goal.goal_atoms, boost::hana::type<R> {}), boost::hana::type<P> {});
}

template const GroundAtomList<StaticTag>& ProblemImpl::get_goal_atoms<PositiveTag, StaticTag>() const;
template const GroundAtomList<FluentTag>& ProblemImpl::get_goal_atoms<PositiveTag, FluentTag>() const;
template const GroundAtomList<DerivedTag>& ProblemImpl::get_goal_atoms<PositiveTag, DerivedTag>() const;
template const GroundAtomList<StaticTag>& ProblemImpl::get_goal_atoms<NegativeTag, StaticTag>() const;
template const GroundAtomList<FluentTag>& ProblemImpl::get_goal_atoms<NegativeTag, FluentTag>() const;
template const GroundAtomList<DerivedTag>& ProblemImpl::get_goal_atoms<NegativeTag, DerivedTag>() const;

const HanaContainer<GroundAtomLists<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag>& ProblemImpl::get_hana_goal_atoms() const
{
    return m_details.goal.goal_atoms;
}

template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
const FlatBitset& ProblemImpl::get_goal_atoms_bitset() const
{
    return boost::hana::at_key(boost::hana::at_key(m_details.goal.goal_atoms_bitset, boost::hana::type<R> {}), boost::hana::type<P> {});
}

template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<PositiveTag, StaticTag>() const;
template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<PositiveTag, FluentTag>() const;
template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<PositiveTag, DerivedTag>() const;
template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<NegativeTag, StaticTag>() const;
template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<NegativeTag, FluentTag>() const;
template const FlatBitset& ProblemImpl::get_goal_atoms_bitset<NegativeTag, DerivedTag>() const;

const HanaContainer<FlatBitsets<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag>& ProblemImpl::get_hana_goal_atoms_bitset() const
{
    return m_details.goal.goal_atoms_bitset;
}

template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
const FlatIndexList& ProblemImpl::get_goal_atoms_indices() const
{
    return boost::hana::at_key(boost::hana::at_key(m_details.goal.goal_atoms_indices, boost::hana::type<R> {}), boost::hana::type<P> {});
}

template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<PositiveTag, StaticTag>() const;
template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<PositiveTag, FluentTag>() const;
template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<PositiveTag, DerivedTag>() const;
template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<NegativeTag, StaticTag>() const;
template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<NegativeTag, FluentTag>() const;
template const FlatIndexList& ProblemImpl::get_goal_atoms_indices<NegativeTag, DerivedTag>() const;

const HanaContainer<FlatIndexLists<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag>& ProblemImpl::get_hana_goal_atoms_indices() const
{
    return m_details.goal.goal_atoms_indices;
}

/* Axioms */

const std::vector<AxiomPartition>& ProblemImpl::get_problem_and_domain_axiom_partitioning() const
{
    return m_details.axiom.problem_and_domain_axiom_partitioning;
}

/**
 * Modifiers
 */

/* Grounding */

// Atom

template<IsStaticOrFluentOrDerivedTag P>
GroundAtom<P> ProblemImpl::get_or_create_ground_atom(Predicate<P> predicate, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_atom(predicate, binding);
}

template GroundAtom<StaticTag> ProblemImpl::get_or_create_ground_atom(Predicate<StaticTag> predicate, const ObjectList& binding);
template GroundAtom<FluentTag> ProblemImpl::get_or_create_ground_atom(Predicate<FluentTag> predicate, const ObjectList& binding);
template GroundAtom<DerivedTag> ProblemImpl::get_or_create_ground_atom(Predicate<DerivedTag> predicate, const ObjectList& binding);

// Literal

template<IsStaticOrFluentOrDerivedTag P>
GroundLiteral<P> ProblemImpl::ground(Literal<P> literal, const ObjectList& binding)
{
    return m_repositories.ground(literal, binding);
}

template GroundLiteral<StaticTag> ProblemImpl::ground(Literal<StaticTag> literal, const ObjectList& binding);
template GroundLiteral<FluentTag> ProblemImpl::ground(Literal<FluentTag> literal, const ObjectList& binding);
template GroundLiteral<DerivedTag> ProblemImpl::ground(Literal<DerivedTag> literal, const ObjectList& binding);

// Function

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> ProblemImpl::ground(Function<F> function, const ObjectList& binding)
{
    return m_repositories.ground(function, binding);
}

template GroundFunction<StaticTag> ProblemImpl::ground(Function<StaticTag> function, const ObjectList& binding);
template GroundFunction<FluentTag> ProblemImpl::ground(Function<FluentTag> function, const ObjectList& binding);
template GroundFunction<AuxiliaryTag> ProblemImpl::ground(Function<AuxiliaryTag> function, const ObjectList& binding);

// FunctionExpression

GroundFunctionExpression ProblemImpl::ground(FunctionExpression fexpr, const ObjectList& binding) { return m_repositories.ground(fexpr, binding); }

// NumericConstraint

GroundNumericConstraint ProblemImpl::ground(NumericConstraint numeric_constraint, const ObjectList& binding)
{
    return m_repositories.ground(numeric_constraint, binding);
}

// NumericEffect

template<IsFluentOrAuxiliaryTag F>
GroundNumericEffect<F> ProblemImpl::ground(NumericEffect<F> numeric_effect, const ObjectList& binding)
{
    return m_repositories.ground(numeric_effect, binding);
}

template GroundNumericEffect<FluentTag> ProblemImpl::ground(NumericEffect<FluentTag> numeric_effect, const ObjectList& binding);
template GroundNumericEffect<AuxiliaryTag> ProblemImpl::ground(NumericEffect<AuxiliaryTag> numeric_effect, const ObjectList& binding);

// Action

static void ground_and_fill_vector(ProblemImpl& problem,
                                   const NumericConstraintList& numeric_constraints,
                                   const ObjectList& binding,
                                   GroundNumericConstraintList& ref_numeric_constraints)
{
    for (const auto& condition : numeric_constraints)
    {
        ref_numeric_constraints.push_back(problem.ground(condition, binding));
    }
}

static void ground_and_fill_vector(ProblemImpl& problem,
                                   const NumericEffectList<FluentTag>& numeric_effects,
                                   const ObjectList& binding,
                                   GroundNumericEffectList<FluentTag>& ref_numeric_effects)
{
    for (const auto& effect : numeric_effects)
    {
        ref_numeric_effects.push_back(problem.ground(effect, binding));
    }
}

static void ground_and_fill_optional(ProblemImpl& problem,
                                     const std::optional<NumericEffect<AuxiliaryTag>>& numeric_effect,
                                     const ObjectList& binding,
                                     std::optional<GroundNumericEffect<AuxiliaryTag>>& ref_numeric_effect)
{
    if (numeric_effect.has_value())
    {
        assert(!ref_numeric_effect.has_value());
        ref_numeric_effect = problem.ground(numeric_effect.value(), binding);
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void ground_and_fill_vector(ProblemImpl& problem,
                                   const std::vector<Literal<P>>& literals,
                                   FlatIndexList& ref_positive_indices,
                                   FlatIndexList& ref_negative_indices,
                                   const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = problem.ground(literal, binding);

        if (grounded_literal->get_polarity())
        {
            ref_positive_indices.push_back(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_negative_indices.push_back(grounded_literal->get_atom()->get_index());
        }
    }
    std::sort(ref_positive_indices.uncompressed_begin(), ref_positive_indices.uncompressed_end());
    std::sort(ref_negative_indices.uncompressed_begin(), ref_negative_indices.uncompressed_end());
}

template void ground_and_fill_vector(ProblemImpl& problem,
                                     const std::vector<Literal<StaticTag>>& literals,
                                     FlatIndexList& ref_positive_indices,
                                     FlatIndexList& ref_negative_indices,
                                     const ObjectList& binding);
template void ground_and_fill_vector(ProblemImpl& problem,
                                     const std::vector<Literal<FluentTag>>& literals,
                                     FlatIndexList& ref_positive_indices,
                                     FlatIndexList& ref_negative_indices,
                                     const ObjectList& binding);
template void ground_and_fill_vector(ProblemImpl& problem,
                                     const std::vector<Literal<DerivedTag>>& literals,
                                     FlatIndexList& ref_positive_indices,
                                     FlatIndexList& ref_negative_indices,
                                     const ObjectList& binding);

GroundConjunctiveCondition ProblemImpl::ground(ConjunctiveCondition conjunctive_condition, const ObjectList& binding)
{
    auto positive_index_list = FlatIndexList {};
    auto negative_index_list = FlatIndexList {};

    positive_index_list.clear();
    negative_index_list.clear();
    ground_and_fill_vector(*this, conjunctive_condition->get_literals<StaticTag>(), positive_index_list, negative_index_list, binding);
    positive_index_list.compress();
    negative_index_list.compress();
    const auto positive_static_precondition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_static_precondition_ptr = get_or_create_index_list(negative_index_list).first;

    positive_index_list.clear();
    negative_index_list.clear();
    ground_and_fill_vector(*this, conjunctive_condition->get_literals<FluentTag>(), positive_index_list, negative_index_list, binding);
    positive_index_list.compress();
    negative_index_list.compress();
    const auto positive_fluent_precondition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_fluent_precondition_ptr = get_or_create_index_list(negative_index_list).first;

    positive_index_list.clear();
    negative_index_list.clear();
    ground_and_fill_vector(*this, conjunctive_condition->get_literals<DerivedTag>(), positive_index_list, negative_index_list, binding);
    positive_index_list.compress();
    negative_index_list.compress();
    const auto positive_derived_precondition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_derived_precondition_ptr = get_or_create_index_list(negative_index_list).first;

    auto numeric_constraints = GroundNumericConstraintList {};
    ground_and_fill_vector(*this, conjunctive_condition->get_numeric_constraints(), binding, numeric_constraints);

    return m_repositories.get_or_create_ground_conjunctive_condition(
        boost::hana::make_map(
            boost::hana::make_pair(boost::hana::type<PositiveTag> {},
                                   boost::hana::make_map(boost::hana::make_pair(boost::hana::type<StaticTag> {}, positive_static_precondition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<FluentTag> {}, positive_fluent_precondition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<DerivedTag> {}, positive_derived_precondition_ptr))),
            boost::hana::make_pair(boost::hana::type<NegativeTag> {},
                                   boost::hana::make_map(boost::hana::make_pair(boost::hana::type<StaticTag> {}, negative_static_precondition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<FluentTag> {}, negative_fluent_precondition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<DerivedTag> {}, negative_derived_precondition_ptr)))),
        std::move(numeric_constraints));
}

GroundConjunctiveEffect ProblemImpl::ground(ConjunctiveEffect conjunctive_effect, const ObjectList& binding)
{
    auto positive_index_list = FlatIndexList {};
    auto negative_index_list = FlatIndexList {};

    positive_index_list.clear();
    negative_index_list.clear();
    ground_and_fill_vector(*this, conjunctive_effect->get_literals(), positive_index_list, negative_index_list, binding);
    positive_index_list.compress();
    negative_index_list.compress();
    const auto positive_effect_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_effect_ptr = get_or_create_index_list(negative_index_list).first;

    /* Conjunctive numerical effects */
    auto fluent_numerical_effects = GroundNumericEffectList<FluentTag> {};
    auto auxiliary_numerical_effect = std::optional<GroundNumericEffect<AuxiliaryTag>> { std::nullopt };
    ground_and_fill_vector(*this, conjunctive_effect->get_fluent_numeric_effects(), binding, fluent_numerical_effects);
    ground_and_fill_optional(*this, conjunctive_effect->get_auxiliary_numeric_effect(), binding, auxiliary_numerical_effect);

    return m_repositories.get_or_create_ground_conjunctive_effect(
        boost::hana::make_map(boost::hana::make_pair(boost::hana::type<PositiveTag> {}, positive_effect_ptr),
                              boost::hana::make_pair(boost::hana::type<NegativeTag> {}, negative_effect_ptr)),
        std::move(fluent_numerical_effects),
        std::move(auxiliary_numerical_effect));
}

GroundConditionalEffect ProblemImpl::ground(ConditionalEffect conditional_effect, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_conditional_effect(ground(conditional_effect->get_conjunctive_condition(), binding),
                                                                  ground(conditional_effect->get_conjunctive_effect(), binding));
}

GroundAction ProblemImpl::ground(Action action, const ObjectList& binding)
{
    /* 1. Check if grounding is cached */

    auto& grounding_tables = boost::hana::at_key(m_details.grounding.grounding_tables, boost::hana::type<GroundAction> {});

    const auto action_index = action->get_index();
    if (action_index >= grounding_tables.size())
    {
        grounding_tables.resize(action_index + 1);
    }
    auto& grounding_table = grounding_tables.at(action_index);

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    auto ground_conjunctive_condition = ground(action->get_conjunctive_condition(), binding);

    auto ground_conditional_effects = GroundConditionalEffectList {};

    /* Conditional effects */

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    static thread_local auto s_binding_cond_effect = ObjectList {};
    s_binding_cond_effect = binding;

    const auto& action_info = m_details.grounding.get_action_infos().at(action->get_index());

    const auto num_lifted_cond_effects = action->get_conditional_effects().size();
    if (num_lifted_cond_effects > 0)
    {
        for (size_t i = 0; i < num_lifted_cond_effects; ++i)
        {
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& objects_by_parameter_index = action_info.conditional_effect_infos.at(i).candidate_variable_bindings;

                // Resize binding to have additional space for all variables in quantified effect.
                s_binding_cond_effect.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create resulting conditional effect binding.
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        s_binding_cond_effect[binding.size() + pos] = m_repositories.get_object(binding_cond[pos]);
                    }

                    ground_conditional_effects.push_back(ground(lifted_cond_effect, s_binding_cond_effect));
                }
            }
            else
            {
                ground_conditional_effects.push_back(ground(lifted_cond_effect, binding));
            }
        }
    }

    auto grounded_action = m_repositories.get_or_create_ground_action(action, binding, ground_conjunctive_condition, std::move(ground_conditional_effects));

    /* 3. Insert to groundings table */

    grounding_table.emplace(binding, GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

// Axiom

GroundAxiom ProblemImpl::ground(Axiom axiom, const ObjectList& binding)
{
    /* 1. Check if grounding is cached */

    auto& grounding_tables = boost::hana::at_key(m_details.grounding.grounding_tables, boost::hana::type<GroundAxiom> {});

    const auto axiom_index = axiom->get_index();
    if (axiom_index >= grounding_tables.size())
    {
        grounding_tables.resize(axiom_index + 1);
    }
    auto& grounding_table = grounding_tables.at(axiom_index);

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    auto grounded_conjunctive_condition = ground(axiom->get_conjunctive_condition(), binding);
    auto grounded_literal = ground(axiom->get_literal(), binding);

    auto grounded_axiom = m_repositories.get_or_create_ground_axiom(axiom, binding, grounded_conjunctive_condition, grounded_literal);

    /* 3. Insert to groundings table */

    grounding_table.emplace(binding, GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

/* Lifting */

Variable ProblemImpl::get_or_create_variable(std::string name, size_t parameter_index)
{
    return m_repositories.get_or_create_variable(std::move(name), std::move(parameter_index));
}

Parameter ProblemImpl::get_or_create_parameter(Variable variable, TypeList types) { return m_repositories.get_or_create_parameter(variable, std::move(types)); }

Term ProblemImpl::get_or_create_term(Variable variable) { return m_repositories.get_or_create_term(variable); }

Term ProblemImpl::get_or_create_term(Object object) { return m_repositories.get_or_create_term(object); }

template<IsStaticOrFluentOrDerivedTag P>
Atom<P> ProblemImpl::get_or_create_atom(Predicate<P> predicate, TermList terms)
{
    return m_repositories.get_or_create_atom(std::move(predicate), std::move(terms));
}

template Atom<StaticTag> ProblemImpl::get_or_create_atom(Predicate<StaticTag> predicate, TermList terms);
template Atom<FluentTag> ProblemImpl::get_or_create_atom(Predicate<FluentTag> predicate, TermList terms);
template Atom<DerivedTag> ProblemImpl::get_or_create_atom(Predicate<DerivedTag> predicate, TermList terms);

template<IsStaticOrFluentOrDerivedTag P>
Literal<P> ProblemImpl::get_or_create_literal(bool polarity, Atom<P> atom)
{
    return m_repositories.get_or_create_literal(polarity, atom);
}

template Literal<StaticTag> ProblemImpl::get_or_create_literal(bool polarity, Atom<StaticTag> atom);
template Literal<FluentTag> ProblemImpl::get_or_create_literal(bool polarity, Atom<FluentTag> atom);
template Literal<DerivedTag> ProblemImpl::get_or_create_literal(bool polarity, Atom<DerivedTag> atom);

template<IsStaticOrFluentOrAuxiliaryTag F>
Function<F> ProblemImpl::get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms)
{
    return m_repositories.get_or_create_function(function_skeleton, std::move(terms));
}

template Function<StaticTag> ProblemImpl::get_or_create_function(FunctionSkeleton<StaticTag> function_skeleton, TermList terms);
template Function<FluentTag> ProblemImpl::get_or_create_function(FunctionSkeleton<FluentTag> function_skeleton, TermList terms);
template Function<AuxiliaryTag> ProblemImpl::get_or_create_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, TermList terms);

FunctionExpressionNumber ProblemImpl::get_or_create_function_expression_number(double number)
{
    return m_repositories.get_or_create_function_expression_number(number);
}

FunctionExpressionBinaryOperator ProblemImpl::get_or_create_function_expression_binary_operator(loki::BinaryOperatorEnum binary_operator,
                                                                                                FunctionExpression left_function_expression,
                                                                                                FunctionExpression right_function_expression)
{
    return m_repositories.get_or_create_function_expression_binary_operator(binary_operator, left_function_expression, right_function_expression);
}

FunctionExpressionMultiOperator ProblemImpl::get_or_create_function_expression_multi_operator(loki::MultiOperatorEnum multi_operator,
                                                                                              FunctionExpressionList function_expressions)
{
    return m_repositories.get_or_create_function_expression_multi_operator(multi_operator, std::move(function_expressions));
}

FunctionExpressionMinus ProblemImpl::get_or_create_function_expression_minus(FunctionExpression function_expression)
{
    return m_repositories.get_or_create_function_expression_minus(function_expression);
}

template<IsStaticOrFluentTag F>
FunctionExpressionFunction<F> ProblemImpl::get_or_create_function_expression_function(Function<F> function)
{
    return m_repositories.get_or_create_function_expression_function(function);
}

template FunctionExpressionFunction<StaticTag> ProblemImpl::get_or_create_function_expression_function(Function<StaticTag> function);
template FunctionExpressionFunction<FluentTag> ProblemImpl::get_or_create_function_expression_function(Function<FluentTag> function);

FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionNumber fexpr)
{
    return m_repositories.get_or_create_function_expression(fexpr);
}

FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionBinaryOperator fexpr)
{
    return m_repositories.get_or_create_function_expression(fexpr);
}

FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionMultiOperator fexpr)
{
    return m_repositories.get_or_create_function_expression(fexpr);
}

FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionMinus fexpr)
{
    return m_repositories.get_or_create_function_expression(fexpr);
}

template<IsStaticOrFluentTag F>
FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionFunction<F> fexpr)
{
    return m_repositories.get_or_create_function_expression(fexpr);
}

template FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionFunction<StaticTag> fexpr);
template FunctionExpression ProblemImpl::get_or_create_function_expression(FunctionExpressionFunction<FluentTag> fexpr);

NumericConstraint ProblemImpl::get_or_create_numeric_constraint(loki::BinaryComparatorEnum binary_comparator,
                                                                FunctionExpression left_function_expression,
                                                                FunctionExpression right_function_expression,
                                                                TermList terms)
{
    return m_repositories.get_or_create_numeric_constraint(binary_comparator, left_function_expression, right_function_expression, std::move(terms));
}

ConjunctiveCondition ProblemImpl::get_or_create_conjunctive_condition(ParameterList parameters,
                                                                      LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                                      NumericConstraintList numeric_constraints)
{
    return m_repositories.get_or_create_conjunctive_condition(std::move(parameters), std::move(literals), std::move(numeric_constraints));
}

GroundConjunctiveCondition ProblemImpl::get_or_create_ground_conjunctive_condition(GroundLiteralList<StaticTag> static_literals,
                                                                                   GroundLiteralList<FluentTag> fluent_literals,
                                                                                   GroundLiteralList<DerivedTag> derived_literals)
{
    auto positive_index_list = FlatIndexList {};
    auto negative_index_list = FlatIndexList {};

    auto populate_index_lists = [&positive_index_list, &negative_index_list](const auto& literals)
    {
        positive_index_list.clear();
        negative_index_list.clear();
        for (const auto& literal : literals)
        {
            if (literal->get_polarity())
            {
                positive_index_list.push_back(literal->get_atom()->get_index());
            }
            else
            {
                negative_index_list.push_back(literal->get_atom()->get_index());
            }
        }
        std::sort(positive_index_list.uncompressed_begin(), positive_index_list.uncompressed_end());
        std::sort(negative_index_list.uncompressed_begin(), negative_index_list.uncompressed_end());
        positive_index_list.compress();
        negative_index_list.compress();
    };

    populate_index_lists(static_literals);
    const auto positive_static_condition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_static_condition_ptr = get_or_create_index_list(negative_index_list).first;

    populate_index_lists(fluent_literals);
    const auto positive_fluent_condition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_fluent_condition_ptr = get_or_create_index_list(negative_index_list).first;

    populate_index_lists(derived_literals);
    const auto positive_derived_condition_ptr = get_or_create_index_list(positive_index_list).first;
    const auto negative_derived_condition_ptr = get_or_create_index_list(negative_index_list).first;

    return m_repositories.get_or_create_ground_conjunctive_condition(
        boost::hana::make_map(
            boost::hana::make_pair(boost::hana::type<PositiveTag> {},
                                   boost::hana::make_map(boost::hana::make_pair(boost::hana::type<StaticTag> {}, positive_static_condition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<FluentTag> {}, positive_fluent_condition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<DerivedTag> {}, positive_derived_condition_ptr))),
            boost::hana::make_pair(boost::hana::type<NegativeTag> {},
                                   boost::hana::make_map(boost::hana::make_pair(boost::hana::type<StaticTag> {}, negative_static_condition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<FluentTag> {}, negative_fluent_condition_ptr),
                                                         boost::hana::make_pair(boost::hana::type<DerivedTag> {}, negative_derived_condition_ptr)))),
        GroundNumericConstraintList {});
}

const problem::ActionGroundingInfoList& problem::GroundingDetails::get_action_infos() const
{
    if (!action_infos.has_value())
    {
        action_infos = ActionGroundingInfoList {};

        for (const auto& action : parent->get_domain()->get_actions())
        {
            auto conditional_effect_infos = ConditionalEffectGroundingInfoList {};

            for (const auto& conditional_effect : action->get_conditional_effects())
            {
                conditional_effect_infos.emplace_back(
                    std::get<2>(StaticConsistencyGraph::compute_vertices(*parent,
                                                                         action->get_arity(),
                                                                         action->get_arity() + conditional_effect->get_arity(),
                                                                         conditional_effect->get_conjunctive_condition()->get_literals<StaticTag>())));
            }

            action_infos->emplace_back(std::move(conditional_effect_infos));
        }
    }

    return *action_infos;
}

/**
 * Details
 */

problem::ObjectDetails::ObjectDetails() : parent(nullptr), name_to_object(), name_to_problem_or_domain_object() {}

problem::ObjectDetails::ObjectDetails(const ProblemImpl& problem) : parent(&problem), name_to_object(), name_to_problem_or_domain_object()
{
    for (const auto& object : problem.get_objects())
    {
        name_to_object.emplace(object->get_name(), object);
    }
    for (const auto& object : problem.get_problem_and_domain_objects())
    {
        name_to_problem_or_domain_object.emplace(object->get_name(), object);
    }
}

problem::PredicateDetails::PredicateDetails() : parent(nullptr), name_to_derived_predicate(), name_to_problem_or_domain_derived_predicate() {}

problem::PredicateDetails::PredicateDetails(const ProblemImpl& problem) :
    parent(&problem),
    name_to_derived_predicate(),
    name_to_problem_or_domain_derived_predicate()
{
    for (const auto& predicate : problem.get_derived_predicates())
    {
        name_to_derived_predicate.emplace(predicate->get_name(), predicate);
    }
    for (const auto& predicate : problem.get_problem_and_domain_derived_predicates())
    {
        name_to_problem_or_domain_derived_predicate.emplace(predicate->get_name(), predicate);
    }
}

problem::InitialDetails::InitialDetails() : parent(nullptr) {}

problem::InitialDetails::InitialDetails(const ProblemImpl& problem) :
    parent(&problem),
    positive_static_initial_atoms(to_ground_atoms(problem.get_initial_literals<StaticTag>())),
    positive_static_initial_atoms_bitset(),
    positive_static_initial_atoms_indices(),
    positive_static_initial_predicate_assignment_sets(
        PredicateAssignmentSets<StaticTag>(problem.get_problem_and_domain_objects(), problem.get_domain()->get_predicates<StaticTag>())),
    static_initial_function_skeleton_assignment_sets(
        FunctionSkeletonAssignmentSets<StaticTag>(problem.get_problem_and_domain_objects(), problem.get_domain()->get_function_skeletons<StaticTag>())),
    positive_fluent_initial_atoms(to_ground_atoms(problem.get_initial_literals<FluentTag>())),
    initial_function_to_value()
{
    for (const auto& literal : problem.get_initial_literals<StaticTag>())
    {
        if (literal->get_polarity())
        {
            positive_static_initial_atoms_bitset.set(literal->get_atom()->get_index());
        }
    }

    std::transform(positive_static_initial_atoms_bitset.begin(),
                   positive_static_initial_atoms_bitset.end(),
                   std::back_inserter(positive_static_initial_atoms_indices),
                   [](unsigned long val) { return static_cast<Index>(val); });

    positive_static_initial_predicate_assignment_sets.insert_ground_atoms(positive_static_initial_atoms);

    // As the ground functions in the goal might not necessarily be defined, we fill the gaps with undefined.
    // In principle, we could compress and define those values during search when applying an action that assigns it.
    boost::hana::for_each(problem.get_hana_initial_function_values(),
                          [this](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& function_values = boost::hana::second(pair);

                              for (const auto& function_value : function_values)
                              {
                                  const auto function = function_value->get_function();
                                  const auto index = function->get_index();
                                  const auto value = function_value->get_number();

                                  auto& function_to_value = boost::hana::at_key(initial_function_to_value, key);

                                  if (index >= function_to_value.size())
                                  {
                                      function_to_value.resize(index + 1, UNDEFINED_CONTINUOUS_COST);
                                  }
                                  function_to_value[index] = value;
                              }
                          });

    auto static_functions = GroundFunctionList<StaticTag> {};
    for (const auto& function_value : problem.get_initial_function_values<StaticTag>())
    {
        const auto function = function_value->get_function();
        const auto index = function->get_index();
        if (index >= static_functions.size())
        {
            static_functions.resize(index + 1, nullptr);
        }
        static_functions[index] = function;
    }

    const auto& static_function_to_values = boost::hana::at_key(initial_function_to_value, boost::hana::type<StaticTag> {});

    static_initial_function_skeleton_assignment_sets.insert_ground_function_values(static_functions, static_function_to_values);
}

problem::GoalDetails::GoalDetails() : parent(nullptr) {}

problem::GoalDetails::GoalDetails(const ProblemImpl& problem, const InitialDetails& initial) :
    parent(&problem),
    m_static_goal_holds(false),
    goal_atoms(),
    goal_atoms_bitset(),
    goal_atoms_indices()
{
    m_static_goal_holds = true;
    for (const auto& literal : problem.get_goal_condition<StaticTag>())
    {
        if (literal->get_polarity() != initial.positive_static_initial_atoms_bitset.get(literal->get_atom()->get_index()))
        {
            m_static_goal_holds = false;
        }
    }

    boost::hana::for_each(
        problem.get_hana_goal_condition(),
        [this](auto&& pair)
        {
            const auto& key = boost::hana::first(pair);
            const auto& value = boost::hana::second(pair);

            auto& negative_goal_atoms = boost::hana::at_key(boost::hana::at_key(goal_atoms, boost::hana::type<NegativeTag> {}), key);
            auto& positive_goal_atoms = boost::hana::at_key(boost::hana::at_key(goal_atoms, boost::hana::type<PositiveTag> {}), key);
            auto& negative_goal_atoms_bitset = boost::hana::at_key(boost::hana::at_key(goal_atoms_bitset, boost::hana::type<NegativeTag> {}), key);
            auto& positive_goal_atoms_bitset = boost::hana::at_key(boost::hana::at_key(goal_atoms_bitset, boost::hana::type<PositiveTag> {}), key);
            auto& negative_goal_atoms_indices = boost::hana::at_key(boost::hana::at_key(goal_atoms_indices, boost::hana::type<NegativeTag> {}), key);
            auto& positive_goal_atoms_indices = boost::hana::at_key(boost::hana::at_key(goal_atoms_indices, boost::hana::type<PositiveTag> {}), key);

            negative_goal_atoms = filter_ground_atoms(value, false);
            positive_goal_atoms = filter_ground_atoms(value, true);

            for (const auto& literal : value)
            {
                if (literal->get_polarity())
                {
                    positive_goal_atoms_bitset.set(literal->get_atom()->get_index());
                }
                else
                {
                    negative_goal_atoms_bitset.set(literal->get_atom()->get_index());
                }
            }

            for (const auto& atom_index : negative_goal_atoms_bitset)
            {
                negative_goal_atoms_indices.push_back(atom_index);
            }
            for (const auto& atom_index : positive_goal_atoms_bitset)
            {
                positive_goal_atoms_indices.push_back(atom_index);
            }
        });
}

problem::AxiomDetails::AxiomDetails() : parent(nullptr) {}

problem::AxiomDetails::AxiomDetails(const ProblemImpl& problem) : parent(&problem), problem_and_domain_axiom_partitioning()
{
    problem_and_domain_axiom_partitioning =
        compute_axiom_partitioning(problem.get_problem_and_domain_axioms(), problem.get_problem_and_domain_derived_predicates());
}

problem::GroundingDetails::GroundingDetails() : parent(nullptr) {}

problem::GroundingDetails::GroundingDetails(const ProblemImpl& problem) : parent(&problem), action_infos(std::nullopt) {}

problem::Details::Details() : parent(nullptr) {}

problem::Details::Details(const ProblemImpl& problem) :
    parent(&problem),
    objects(problem),
    predicates(problem),
    initial(problem),
    goal(problem, initial),
    axiom(problem),
    grounding(problem)
{
}

/* Printing */
std::ostream& operator<<(std::ostream& out, const ProblemImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Problem element)
{
    write(*element, StringFormatter(), out);
    return out;
}

}
