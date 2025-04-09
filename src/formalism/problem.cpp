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
#include "mimir/formalism/translator.hpp"
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
    m_details()
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
    /* Parse */
    auto parser = Parser(domain_filepath, options);
    auto problem = parser.parse_problem(problem_filepath, options);
    /* Translate */
    auto translator = Translator(problem->get_domain());
    return translator.translate(problem);
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

/**
 * Additional members
 */

/* Initial state */

const GroundAtomList<StaticTag>& ProblemImpl::get_static_initial_atoms() const { return m_details.initial.positive_static_initial_atoms; }

const FlatBitset& ProblemImpl::get_static_initial_positive_atoms_bitset() const { return m_details.initial.positive_static_initial_atoms_bitset; }

const FlatIndexList& ProblemImpl::get_static_initial_positive_atoms_indices() const
{
    assert(std::is_sorted(m_details.initial.positive_static_initial_atoms_indices.begin(), m_details.initial.positive_static_initial_atoms_indices.end()));
    return m_details.initial.positive_static_initial_atoms_indices;
}

const AssignmentSet<StaticTag>& ProblemImpl::get_static_assignment_set() const { return m_details.initial.positive_static_initial_assignment_set; }

const NumericAssignmentSet<StaticTag>& ProblemImpl::get_static_initial_numeric_assignment_set() const
{
    return m_details.initial.static_initial_numeric_assignment_set;
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

bool ProblemImpl::static_literal_holds(const GroundLiteral<StaticTag> literal) const
{
    return (literal->get_polarity() != get_static_initial_positive_atoms_bitset().get(literal->get_atom()->get_index()));
}

template<IsStaticOrFluentOrDerivedTag P>
const GroundAtomList<P>& ProblemImpl::get_positive_goal_atoms() const
{
    return boost::hana::at_key(m_details.goal.positive_goal_atoms, boost::hana::type<P> {});
}

template const GroundAtomList<StaticTag>& ProblemImpl::get_positive_goal_atoms<StaticTag>() const;
template const GroundAtomList<FluentTag>& ProblemImpl::get_positive_goal_atoms<FluentTag>() const;
template const GroundAtomList<DerivedTag>& ProblemImpl::get_positive_goal_atoms<DerivedTag>() const;

const GroundAtomLists<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_positive_goal_atoms() const { return m_details.goal.positive_goal_atoms; }

template<IsStaticOrFluentOrDerivedTag P>
const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset() const
{
    return boost::hana::at_key(m_details.goal.positive_goal_atoms_bitset, boost::hana::type<P> {});
}

template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<StaticTag>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<FluentTag>() const;
template const FlatBitset& ProblemImpl::get_positive_goal_atoms_bitset<DerivedTag>() const;

const FlatBitsets<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_positive_goal_atoms_bitset() const
{
    return m_details.goal.positive_goal_atoms_bitset;
}

template<IsStaticOrFluentOrDerivedTag P>
const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices() const
{
    return boost::hana::at_key(m_details.goal.positive_goal_atoms_indices, boost::hana::type<P> {});
}

template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<StaticTag>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<FluentTag>() const;
template const FlatIndexList& ProblemImpl::get_positive_goal_atoms_indices<DerivedTag>() const;

const FlatIndexLists<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_positive_goal_atoms_indices() const
{
    return m_details.goal.positive_goal_atoms_indices;
}

template<IsStaticOrFluentOrDerivedTag P>
const GroundAtomList<P>& ProblemImpl::get_negative_goal_atoms() const
{
    return boost::hana::at_key(m_details.goal.negative_goal_atoms, boost::hana::type<P> {});
}

template const GroundAtomList<StaticTag>& ProblemImpl::get_negative_goal_atoms<StaticTag>() const;
template const GroundAtomList<FluentTag>& ProblemImpl::get_negative_goal_atoms<FluentTag>() const;
template const GroundAtomList<DerivedTag>& ProblemImpl::get_negative_goal_atoms<DerivedTag>() const;

const GroundAtomLists<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_negative_goal_atoms() const { return m_details.goal.negative_goal_atoms; }

template<IsStaticOrFluentOrDerivedTag P>
const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset() const
{
    return boost::hana::at_key(m_details.goal.negative_goal_atoms_bitset, boost::hana::type<P> {});
}

template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<StaticTag>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<FluentTag>() const;
template const FlatBitset& ProblemImpl::get_negative_goal_atoms_bitset<DerivedTag>() const;

const FlatBitsets<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_negative_goal_atoms_bitset() const
{
    return m_details.goal.negative_goal_atoms_bitset;
}

template<IsStaticOrFluentOrDerivedTag P>
const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices() const
{
    return boost::hana::at_key(m_details.goal.negative_goal_atoms_indices, boost::hana::type<P> {});
}

template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<StaticTag>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<FluentTag>() const;
template const FlatIndexList& ProblemImpl::get_negative_goal_atoms_indices<DerivedTag>() const;

const FlatIndexLists<StaticTag, FluentTag, DerivedTag>& ProblemImpl::get_hana_negative_goal_atoms_indices() const
{
    return m_details.goal.negative_goal_atoms_indices;
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

// Terms
static void ground_terms(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Object>)
                {
                    out_terms.emplace_back(arg);
                }
                else if constexpr (std::is_same_v<T, Variable>)
                {
                    assert(arg->get_parameter_index() < binding.size());
                    out_terms.emplace_back(binding[arg->get_parameter_index()]);
                }
            },
            term->get_variant());
    }
}

// Atom

template<IsStaticOrFluentOrDerivedTag P>
GroundAtom<P> ProblemImpl::get_or_create_ground_atom(Predicate<P> predicate, const ObjectList& objects)
{
    return m_repositories.get_or_create_ground_atom(predicate, std::move(objects));
}

template GroundAtom<StaticTag> ProblemImpl::get_or_create_ground_atom(Predicate<StaticTag> predicate, const ObjectList& objects);
template GroundAtom<FluentTag> ProblemImpl::get_or_create_ground_atom(Predicate<FluentTag> predicate, const ObjectList& objects);
template GroundAtom<DerivedTag> ProblemImpl::get_or_create_ground_atom(Predicate<DerivedTag> predicate, const ObjectList& objects);

// Literal

template<IsStaticOrFluentOrDerivedTag P>
static void ground_and_fill_bitset(ProblemImpl& problem,
                                   const std::vector<Literal<P>>& literals,
                                   FlatBitset& ref_positive_bitset,
                                   FlatBitset& ref_negative_bitset,
                                   const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = problem.ground(literal, binding);

        if (grounded_literal->get_polarity())
        {
            ref_positive_bitset.set(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_negative_bitset.set(grounded_literal->get_atom()->get_index());
        }
    }
}

template void ground_and_fill_bitset(ProblemImpl& problem,
                                     const std::vector<Literal<StaticTag>>& literals,
                                     FlatBitset& ref_positive_bitset,
                                     FlatBitset& ref_negative_bitset,
                                     const ObjectList& binding);
template void ground_and_fill_bitset(ProblemImpl& problem,
                                     const std::vector<Literal<FluentTag>>& literals,
                                     FlatBitset& ref_positive_bitset,
                                     FlatBitset& ref_negative_bitset,
                                     const ObjectList& binding);
template void ground_and_fill_bitset(ProblemImpl& problem,
                                     const std::vector<Literal<DerivedTag>>& literals,
                                     FlatBitset& ref_positive_bitset,
                                     FlatBitset& ref_negative_bitset,
                                     const ObjectList& binding);

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
    std::sort(ref_positive_indices.begin(), ref_positive_indices.end());
    std::sort(ref_negative_indices.begin(), ref_negative_indices.end());
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

template<IsStaticOrFluentOrDerivedTag P>
GroundLiteral<P> ProblemImpl::ground(Literal<P> literal, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_details.grounding.grounding_tables, boost::hana::type<GroundLiteral<P>> {});

    /* 2. Access the context-independent literal grounding table */
    const auto polarity = literal->get_polarity();
    const auto predicate_index = literal->get_atom()->get_predicate()->get_index();
    auto& polarity_grounding_tables = grounding_tables[polarity];
    if (predicate_index >= polarity_grounding_tables.size())
    {
        polarity_grounding_tables.resize(predicate_index + 1);
    }

    auto& grounding_table = polarity_grounding_tables.at(predicate_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    auto grounded_terms = ObjectList {};
    ground_terms(literal->get_atom()->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the literal */

    auto grounded_atom = m_repositories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = m_repositories.get_or_create_ground_literal(literal->get_polarity(), grounded_atom);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundLiteral<P>(grounded_literal));

    /* 6. Return the resulting ground literal */

    return grounded_literal;
}

template GroundLiteral<StaticTag> ProblemImpl::ground(Literal<StaticTag> literal, const ObjectList& binding);
template GroundLiteral<FluentTag> ProblemImpl::ground(Literal<FluentTag> literal, const ObjectList& binding);
template GroundLiteral<DerivedTag> ProblemImpl::ground(Literal<DerivedTag> literal, const ObjectList& binding);

// Function

template<IsStaticOrFluentOrAuxiliaryTag F>
GroundFunction<F> ProblemImpl::ground(Function<F> function, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_details.grounding.grounding_tables, boost::hana::type<GroundFunction<F>> {});

    /* 2. Access the context-independent function grounding table */
    const auto function_skeleton_index = function->get_function_skeleton()->get_index();
    if (function_skeleton_index >= grounding_tables.size())
    {
        grounding_tables.resize(function_skeleton_index + 1);
    }

    auto& grounding_table = grounding_tables.at(function_skeleton_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    // Note: this is important and saves a lot of memory.
    auto grounded_terms = ObjectList {};
    ground_terms(function->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function */

    const auto grounded_function = m_repositories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundFunction<F>(grounded_function));

    /* 6. Return the resulting ground literal */

    return grounded_function;
}

template GroundFunction<StaticTag> ProblemImpl::ground(Function<StaticTag> function, const ObjectList& binding);
template GroundFunction<FluentTag> ProblemImpl::ground(Function<FluentTag> function, const ObjectList& binding);
template GroundFunction<AuxiliaryTag> ProblemImpl::ground(Function<AuxiliaryTag> function, const ObjectList& binding);

// FunctionExpression

GroundFunctionExpression ProblemImpl::ground(FunctionExpression fexpr, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_details.grounding.grounding_tables, boost::hana::type<GroundFunctionExpression> {});

    /* 2. Access the context-specific fexpr grounding table
     */

    const auto fexpr_index = fexpr->get_index();
    if (fexpr_index >= grounding_tables.size())
    {
        grounding_tables.resize(fexpr_index + 1);
    }

    auto& grounding_table = grounding_tables.at(fexpr_index);

    /* 3. Check if grounding is cached */

    const auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the function expression */
    const auto grounded_fexpr = std::visit(
        [&](auto&& arg) -> GroundFunctionExpression
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>)
            {
                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_number(arg->get_number()));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                const auto op = arg->get_binary_operator();
                const auto ground_lhs = ground(arg->get_left_function_expression(), binding);
                const auto ground_rhs = ground(arg->get_right_function_expression(), binding);

                if (std::holds_alternative<GroundFunctionExpressionNumber>(ground_lhs->get_variant())
                    && std::holds_alternative<GroundFunctionExpressionNumber>(ground_rhs->get_variant()))
                {
                    return m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(
                        evaluate_binary(op,
                                        std::get<GroundFunctionExpressionNumber>(ground_lhs->get_variant())->get_number(),
                                        std::get<GroundFunctionExpressionNumber>(ground_rhs->get_variant())->get_number())));
                }

                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_binary_operator(op, ground_lhs, ground_rhs));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                const auto op = arg->get_multi_operator();
                auto fexpr_numbers = GroundFunctionExpressionList {};
                auto fexpr_others = GroundFunctionExpressionList {};
                for (const auto& child_fexpr : arg->get_function_expressions())
                {
                    const auto ground_child_fexpr = ground(child_fexpr, binding);
                    std::holds_alternative<GroundFunctionExpressionNumber>(ground_child_fexpr->get_variant()) ? fexpr_numbers.push_back(ground_child_fexpr) :
                                                                                                                fexpr_others.push_back(ground_child_fexpr);
                }

                if (!fexpr_numbers.empty())
                {
                    const auto value =
                        std::accumulate(std::next(fexpr_numbers.begin()),  // Start from the second expression
                                        fexpr_numbers.end(),
                                        std::get<GroundFunctionExpressionNumber>(fexpr_numbers.front()->get_variant())->get_number(),  // Initial bounds
                                        [op](const auto& value, const auto& child_expr) {
                                            return evaluate_multi(op, value, std::get<GroundFunctionExpressionNumber>(child_expr->get_variant())->get_number());
                                        });

                    fexpr_others.push_back(
                        m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(value)));
                }

                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_multi_operator(op, fexpr_others));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                const auto ground_fexpr = ground(arg->get_function_expression(), binding);

                return std::holds_alternative<GroundFunctionExpressionNumber>(ground_fexpr->get_variant()) ?
                           m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(
                               -std::get<GroundFunctionExpressionNumber>(ground_fexpr->get_variant())->get_number())) :
                           ground_fexpr;
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<StaticTag>>)
            {
                return m_repositories.get_or_create_ground_function_expression(m_repositories.get_or_create_ground_function_expression_number(
                    get_initial_function_value<StaticTag>(ground(arg->get_function(), binding))));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<FluentTag>>)
            {
                return m_repositories.get_or_create_ground_function_expression(
                    m_repositories.get_or_create_ground_function_expression_function<FluentTag>(ground(arg->get_function(), binding)));
            }
            else
            {
                static_assert(dependent_false<T>::value,
                              "NumericConstraintGrounder::ground(fexpr, binding): Missing implementation for GroundFunctionExpression type.");
            }
        },
        fexpr->get_variant());

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(binding, GroundFunctionExpression(grounded_fexpr));

    /* 6. Return the resulting ground literal */

    return grounded_fexpr;
}

// NumericConstraint
GroundNumericConstraint ProblemImpl::ground(NumericConstraint numeric_constraint, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_numeric_constraint(numeric_constraint->get_binary_comparator(),
                                                                  ground(numeric_constraint->get_left_function_expression(), binding),
                                                                  ground(numeric_constraint->get_right_function_expression(), binding));
}

// NumericEffect
template<IsFluentOrAuxiliaryTag F>
GroundNumericEffect<F> ProblemImpl::ground(NumericEffect<F> numeric_effect, const ObjectList& binding)
{
    return m_repositories.get_or_create_ground_numeric_effect(numeric_effect->get_assign_operator(),
                                                              ground(numeric_effect->get_function(), binding),
                                                              ground(numeric_effect->get_function_expression(), binding));
}

template GroundNumericEffect<FluentTag> ProblemImpl::ground(NumericEffect<FluentTag> numeric_effect, const ObjectList& binding);
template GroundNumericEffect<AuxiliaryTag> ProblemImpl::ground(NumericEffect<AuxiliaryTag> numeric_effect, const ObjectList& binding);

// Action

static void ground_and_fill_vector(ProblemImpl& problem,
                                   const NumericConstraintList& numeric_constraints,
                                   const ObjectList& binding,
                                   FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints)
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
                                     cista::optional<FlatExternalPtr<const GroundNumericEffectImpl<AuxiliaryTag>>>& ref_numeric_effect)
{
    if (numeric_effect.has_value())
    {
        assert(!ref_numeric_effect.has_value());
        ref_numeric_effect = problem.ground(numeric_effect.value(), binding);
    }
}

GroundAction ProblemImpl::ground(Action action, const ObjectList& binding)
{
    /* 1. Check if grounding is cached */

    auto& [action_builder, grounding_table] = m_details.grounding.per_action_data.at(action->get_index());

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    /* Header */

    action_builder.get_index() = m_details.grounding.ground_actions.size();
    action_builder.get_action_index() = action->get_index();
    auto& objects = action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Conjunctive precondition */
    {
        auto& conjunctive_condition = action_builder.get_conjunctive_condition();
        auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<FluentTag>();
        auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<FluentTag>();
        auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<StaticTag>();
        auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<StaticTag>();
        auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<DerivedTag>();
        auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<DerivedTag>();
        auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
        positive_fluent_precondition.clear();
        negative_fluent_precondition.clear();
        positive_static_precondition.clear();
        negative_static_precondition.clear();
        positive_derived_precondition.clear();
        negative_derived_precondition.clear();
        numeric_constraints.clear();
        ground_and_fill_vector(*this,
                               action->get_conjunctive_condition()->get_literals<FluentTag>(),
                               positive_fluent_precondition,
                               negative_fluent_precondition,
                               binding);
        ground_and_fill_vector(*this,
                               action->get_conjunctive_condition()->get_literals<StaticTag>(),
                               positive_static_precondition,
                               negative_static_precondition,
                               binding);
        ground_and_fill_vector(*this,
                               action->get_conjunctive_condition()->get_literals<DerivedTag>(),
                               positive_derived_precondition,
                               negative_derived_precondition,
                               binding);
        positive_fluent_precondition.compress();
        negative_fluent_precondition.compress();
        positive_static_precondition.compress();
        negative_static_precondition.compress();
        positive_derived_precondition.compress();
        negative_derived_precondition.compress();
        ground_and_fill_vector(*this, action->get_conjunctive_condition()->get_numeric_constraints(), binding, numeric_constraints);

        /* Conjunctive propositional effects */
        auto& conjunctive_effect = action_builder.get_conjunctive_effect();
        auto& positive_effect = conjunctive_effect.get_positive_effects();
        auto& negative_effect = conjunctive_effect.get_negative_effects();
        positive_effect.clear();
        negative_effect.clear();
        ground_and_fill_vector(*this, action->get_conjunctive_effect()->get_literals(), positive_effect, negative_effect, binding);
        positive_effect.compress();
        negative_effect.compress();

        /* Conjunctive numerical effects */
        auto& fluent_numerical_effects = conjunctive_effect.get_fluent_numeric_effects();
        auto& auxiliary_numerical_effect = conjunctive_effect.get_auxiliary_numeric_effect();
        fluent_numerical_effects.clear();
        auxiliary_numerical_effect = std::nullopt;
        ground_and_fill_vector(*this, action->get_conjunctive_effect()->get_fluent_numeric_effects(), binding, fluent_numerical_effects);
        ground_and_fill_optional(*this, action->get_conjunctive_effect()->get_auxiliary_numeric_effect(), binding, auxiliary_numerical_effect);
    }

    /* Conditional effects */
    // Fetch data
    auto& cond_effects = action_builder.get_conditional_effects();

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_cond_effect = binding;

    const auto& action_info = m_details.grounding.get_action_infos().at(action->get_index());

    const auto num_lifted_cond_effects = action->get_conditional_effects().size();
    if (num_lifted_cond_effects > 0)
    {
        size_t j = 0;  ///< position in cond_effects

        for (size_t i = 0; i < num_lifted_cond_effects; ++i)
        {
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& objects_by_parameter_index = action_info.conditional_effect_infos.at(i).candidate_variable_bindings;

                // Resize binding to have additional space for all variables in quantified effect.
                binding_cond_effect.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create resulting conditional effect binding.
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        binding_cond_effect[binding.size() + pos] = m_repositories.get_object(binding_cond[pos]);
                    }

                    // Ensure that there is enough space in the action specific builder.
                    // We never clear cond_effects to avoid frequent allocation and deallocation.
                    if (j == cond_effects.size())
                    {
                        cond_effects.resize(j + 1);
                    }
                    auto& cond_effect_j = cond_effects.at(j++);
                    auto& cond_conjunctive_condition_j = cond_effect_j.get_conjunctive_condition();
                    auto& cond_positive_fluent_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<FluentTag>();
                    auto& cond_negative_fluent_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<FluentTag>();
                    auto& cond_positive_static_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<StaticTag>();
                    auto& cond_negative_static_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<StaticTag>();
                    auto& cond_positive_derived_precondition_j = cond_conjunctive_condition_j.get_positive_precondition<DerivedTag>();
                    auto& cond_negative_derived_precondition_j = cond_conjunctive_condition_j.get_negative_precondition<DerivedTag>();
                    auto& cond_numeric_constraints_j = cond_conjunctive_condition_j.get_numeric_constraints();
                    auto& cond_conjunctive_effect_j = cond_effect_j.get_conjunctive_effect();
                    auto& cond_positive_effect_j = cond_conjunctive_effect_j.get_positive_effects();
                    auto& cond_negative_effect_j = cond_conjunctive_effect_j.get_negative_effects();
                    cond_positive_fluent_precondition_j.clear();
                    cond_negative_fluent_precondition_j.clear();
                    cond_positive_static_precondition_j.clear();
                    cond_negative_static_precondition_j.clear();
                    cond_positive_derived_precondition_j.clear();
                    cond_negative_derived_precondition_j.clear();
                    cond_numeric_constraints_j.clear();
                    cond_positive_effect_j.clear();
                    cond_negative_effect_j.clear();

                    /* Propositional precondition */
                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_condition()->get_literals<FluentTag>(),
                                           cond_positive_fluent_precondition_j,
                                           cond_negative_fluent_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_condition()->get_literals<StaticTag>(),
                                           cond_positive_static_precondition_j,
                                           cond_negative_static_precondition_j,
                                           binding_cond_effect);
                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_condition()->get_literals<DerivedTag>(),
                                           cond_positive_derived_precondition_j,
                                           cond_negative_derived_precondition_j,
                                           binding_cond_effect);
                    cond_positive_fluent_precondition_j.compress();
                    cond_negative_fluent_precondition_j.compress();
                    cond_positive_static_precondition_j.compress();
                    cond_negative_static_precondition_j.compress();
                    cond_positive_derived_precondition_j.compress();
                    cond_negative_derived_precondition_j.compress();
                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(),
                                           binding_cond_effect,
                                           cond_numeric_constraints_j);

                    /* Propositional effect */
                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_effect()->get_literals(),
                                           cond_positive_effect_j,
                                           cond_negative_effect_j,
                                           binding_cond_effect);

                    /* Numeric effect */
                    auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect_j.get_fluent_numeric_effects();
                    auto& cond_auxiliary_numerical_effect_j = cond_conjunctive_effect_j.get_auxiliary_numeric_effect();
                    cond_fluent_numerical_effects_j.clear();
                    cond_auxiliary_numerical_effect_j = std::nullopt;

                    ground_and_fill_vector(*this,
                                           lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                           binding_cond_effect,
                                           cond_fluent_numerical_effects_j);
                    ground_and_fill_optional(*this,
                                             lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                             binding_cond_effect,
                                             cond_auxiliary_numerical_effect_j);
                }
            }
            else
            {
                // Ensure that there is enough space in the action specific builder.
                // We never clear cond_effects to avoid frequent allocation and deallocation.
                if (j == cond_effects.size())
                {
                    cond_effects.resize(j + 1);
                }
                auto& cond_effect = cond_effects.at(j++);
                auto& cond_conjunctive_condition = cond_effect.get_conjunctive_condition();
                auto& cond_positive_fluent_precondition = cond_conjunctive_condition.get_positive_precondition<FluentTag>();
                auto& cond_negative_fluent_precondition = cond_conjunctive_condition.get_negative_precondition<FluentTag>();
                auto& cond_positive_static_precondition = cond_conjunctive_condition.get_positive_precondition<StaticTag>();
                auto& cond_negative_static_precondition = cond_conjunctive_condition.get_negative_precondition<StaticTag>();
                auto& cond_positive_derived_precondition = cond_conjunctive_condition.get_positive_precondition<DerivedTag>();
                auto& cond_negative_derived_precondition = cond_conjunctive_condition.get_negative_precondition<DerivedTag>();
                auto& cond_numeric_constraints = cond_conjunctive_condition.get_numeric_constraints();
                auto& cond_conjunctive_effect = cond_effect.get_conjunctive_effect();
                auto& cond_positive_effect = cond_conjunctive_effect.get_positive_effects();
                auto& cond_negative_effect = cond_conjunctive_effect.get_negative_effects();
                cond_positive_fluent_precondition.clear();
                cond_negative_fluent_precondition.clear();
                cond_positive_static_precondition.clear();
                cond_negative_static_precondition.clear();
                cond_positive_derived_precondition.clear();
                cond_negative_derived_precondition.clear();
                cond_numeric_constraints.clear();
                cond_positive_effect.clear();
                cond_negative_effect.clear();

                /* Propositional precondition */
                ground_and_fill_vector(*this,
                                       lifted_cond_effect->get_conjunctive_condition()->get_literals<FluentTag>(),
                                       cond_positive_fluent_precondition,
                                       cond_negative_fluent_precondition,
                                       binding);
                ground_and_fill_vector(*this,
                                       lifted_cond_effect->get_conjunctive_condition()->get_literals<StaticTag>(),
                                       cond_positive_static_precondition,
                                       cond_negative_static_precondition,
                                       binding);
                ground_and_fill_vector(*this,
                                       lifted_cond_effect->get_conjunctive_condition()->get_literals<DerivedTag>(),
                                       cond_positive_derived_precondition,
                                       cond_negative_derived_precondition,
                                       binding);
                cond_positive_fluent_precondition.compress();
                cond_negative_fluent_precondition.compress();
                cond_positive_static_precondition.compress();
                cond_negative_static_precondition.compress();
                cond_positive_derived_precondition.compress();
                cond_negative_derived_precondition.compress();
                ground_and_fill_vector(*this, lifted_cond_effect->get_conjunctive_condition()->get_numeric_constraints(), binding, cond_numeric_constraints);

                /* Propositional effect */
                ground_and_fill_vector(*this,
                                       lifted_cond_effect->get_conjunctive_effect()->get_literals(),
                                       cond_positive_effect,
                                       cond_negative_effect,
                                       binding);

                /* Numeric effect*/
                auto& cond_fluent_numerical_effects_j = cond_conjunctive_effect.get_fluent_numeric_effects();
                auto& cond_auxiliary_numerical_effects_j = cond_conjunctive_effect.get_auxiliary_numeric_effect();
                cond_fluent_numerical_effects_j.clear();
                cond_auxiliary_numerical_effects_j = std::nullopt;

                ground_and_fill_vector(*this,
                                       lifted_cond_effect->get_conjunctive_effect()->get_fluent_numeric_effects(),
                                       binding,
                                       cond_fluent_numerical_effects_j);
                ground_and_fill_optional(*this,
                                         lifted_cond_effect->get_conjunctive_effect()->get_auxiliary_numeric_effect(),
                                         binding,
                                         cond_auxiliary_numerical_effects_j);
            }
        }
    }

    const auto [iter, inserted] = m_details.grounding.ground_actions.insert(action_builder);
    const auto grounded_action = iter->get();
    if (inserted)
    {
        m_details.grounding.ground_actions_by_index.push_back(grounded_action);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(binding, GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

/* Lifting */

Variable ProblemImpl::get_or_create_variable(std::string name, size_t parameter_index)
{
    return m_repositories.get_or_create_variable(std::move(name), std::move(parameter_index));
}

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
Function<F> ProblemImpl::get_or_create_function(FunctionSkeleton<F> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping)
{
    return m_repositories.get_or_create_function(function_skeleton, std::move(terms), std::move(parent_terms_to_terms_mapping));
}

template Function<StaticTag>
ProblemImpl::get_or_create_function(FunctionSkeleton<StaticTag> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping);
template Function<FluentTag>
ProblemImpl::get_or_create_function(FunctionSkeleton<FluentTag> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping);
template Function<AuxiliaryTag>
ProblemImpl::get_or_create_function(FunctionSkeleton<AuxiliaryTag> function_skeleton, TermList terms, IndexList parent_terms_to_terms_mapping);

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

ConjunctiveCondition ProblemImpl::get_or_create_conjunctive_condition(VariableList parameters,
                                                                      LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                                                                      NumericConstraintList numeric_constraints)
{
    return m_repositories.get_or_create_conjunctive_condition(std::move(parameters), std::move(literals), std::move(numeric_constraints));
}

/* Accessors */

const GroundActionList& ProblemImpl::get_ground_actions() const { return m_details.grounding.ground_actions_by_index; }

GroundAction ProblemImpl::get_ground_action(Index action_index) const { return m_details.grounding.ground_actions_by_index.at(action_index); }

size_t ProblemImpl::get_num_ground_actions() const { return m_details.grounding.ground_actions_by_index.size(); }

size_t ProblemImpl::get_estimated_memory_usage_in_bytes_for_actions() const
{
    const auto usage1 = m_details.grounding.ground_actions.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_details.grounding.ground_actions_by_index.capacity() * sizeof(GroundAction);
    auto usage3 = size_t(0);

    for (const auto& [action_builder, grounding_table] : m_details.grounding.per_action_data)
    {
        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}

// Axiom

GroundAxiom ProblemImpl::ground(Axiom axiom, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& [axiom_builder, grounding_table] = m_details.grounding.per_axiom_data.at(axiom->get_index());
    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    /* Header */

    axiom_builder.get_index() = m_details.grounding.ground_axioms.size();
    axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = axiom_builder.get_object_indices();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Precondition */
    auto& conjunctive_condition = axiom_builder.get_conjunctive_condition();
    auto& positive_fluent_precondition = conjunctive_condition.get_positive_precondition<FluentTag>();
    auto& negative_fluent_precondition = conjunctive_condition.get_negative_precondition<FluentTag>();
    auto& positive_static_precondition = conjunctive_condition.get_positive_precondition<StaticTag>();
    auto& negative_static_precondition = conjunctive_condition.get_negative_precondition<StaticTag>();
    auto& positive_derived_precondition = conjunctive_condition.get_positive_precondition<DerivedTag>();
    auto& negative_derived_precondition = conjunctive_condition.get_negative_precondition<DerivedTag>();
    auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    numeric_constraints.clear();
    ground_and_fill_vector(*this,
                           axiom->get_conjunctive_condition()->get_literals<FluentTag>(),
                           positive_fluent_precondition,
                           negative_fluent_precondition,
                           binding);
    ground_and_fill_vector(*this,
                           axiom->get_conjunctive_condition()->get_literals<StaticTag>(),
                           positive_static_precondition,
                           negative_static_precondition,
                           binding);
    ground_and_fill_vector(*this,
                           axiom->get_conjunctive_condition()->get_literals<DerivedTag>(),
                           positive_derived_precondition,
                           negative_derived_precondition,
                           binding);
    positive_fluent_precondition.compress();
    negative_fluent_precondition.compress();
    positive_static_precondition.compress();
    negative_static_precondition.compress();
    positive_derived_precondition.compress();
    negative_derived_precondition.compress();
    ground_and_fill_vector(*this, axiom->get_conjunctive_condition()->get_numeric_constraints(), binding, numeric_constraints);

    /* Effect */

    // The effect literal might only use the first few objects of the complete binding
    // Therefore, we can prevent the literal grounding table from unnecessarily growing
    // by restricting the binding to only the relevant part
    const auto effect_literal_arity = axiom->get_literal()->get_atom()->get_arity();
    const auto is_complete_binding_relevant_for_head = (binding.size() == effect_literal_arity);
    const auto grounded_literal = is_complete_binding_relevant_for_head ?
                                      ground(axiom->get_literal(), binding) :
                                      ground(axiom->get_literal(), ObjectList(binding.begin(), binding.begin() + effect_literal_arity));
    assert(grounded_literal->get_polarity());
    axiom_builder.get_derived_effect().polarity = true;
    axiom_builder.get_derived_effect().atom_index = grounded_literal->get_atom()->get_index();

    const auto [iter, inserted] = m_details.grounding.ground_axioms.insert(axiom_builder);
    const auto grounded_axiom = iter->get();

    if (inserted)
    {
        m_details.grounding.ground_axioms_by_index.push_back(grounded_axiom);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

const GroundAxiomList& ProblemImpl::get_ground_axioms() const { return m_details.grounding.ground_axioms_by_index; }

GroundAxiom ProblemImpl::get_ground_axiom(Index axiom_index) const { return m_details.grounding.ground_axioms_by_index.at(axiom_index); }

size_t ProblemImpl::get_num_ground_axioms() const { return m_details.grounding.ground_axioms_by_index.size(); }

size_t ProblemImpl::get_estimated_memory_usage_in_bytes_for_axioms() const
{
    const auto usage1 = m_details.grounding.ground_axioms.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_details.grounding.ground_axioms_by_index.capacity() * sizeof(GroundAxiom);
    auto usage3 = size_t(0);
    // TODO: add memory usage of per_axiom_data
    for (const auto& [axiom_builder, grounding_table] : m_details.grounding.per_axiom_data)
    {
        // TODO: add memory usage of axiom_builder
        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
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
                auto [vertices_, vertices_by_parameter_index_, objects_by_parameter_index_] =
                    StaticConsistencyGraph::compute_vertices(*parent,
                                                             action->get_arity(),
                                                             action->get_arity() + conditional_effect->get_arity(),
                                                             conditional_effect->get_conjunctive_condition()->get_literals<StaticTag>());

                conditional_effect_infos.emplace_back(std::move(objects_by_parameter_index_));
            }

            action_infos->emplace_back(std::move(conditional_effect_infos));
        }
    }

    return *action_infos;
}

/**
 * Details
 */

problem::InitialDetails::InitialDetails() : parent(nullptr) {}

problem::InitialDetails::InitialDetails(const ProblemImpl& problem) :
    parent(&problem),
    positive_static_initial_atoms(to_ground_atoms(problem.get_initial_literals<StaticTag>())),
    positive_static_initial_atoms_bitset(),
    positive_static_initial_atoms_indices(),
    positive_static_initial_assignment_set(
        AssignmentSet<StaticTag>(problem.get_problem_and_domain_objects().size(), problem.get_domain()->get_predicates<StaticTag>())),
    static_initial_numeric_assignment_set(
        NumericAssignmentSet<StaticTag>(problem.get_problem_and_domain_objects().size(), problem.get_domain()->get_function_skeletons<StaticTag>())),
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

    positive_static_initial_assignment_set.insert_ground_atoms(positive_static_initial_atoms);

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

    static_initial_numeric_assignment_set.insert_ground_function_values(static_functions, static_function_to_values);
}

problem::GoalDetails::GoalDetails() : parent(nullptr) {}

problem::GoalDetails::GoalDetails(const ProblemImpl& problem, const InitialDetails& initial) :
    parent(&problem),
    m_static_goal_holds(false),
    positive_goal_atoms(),
    positive_goal_atoms_bitset(),
    positive_goal_atoms_indices(),
    negative_goal_atoms(),
    negative_goal_atoms_bitset(),
    negative_goal_atoms_indices()
{
    m_static_goal_holds = true;
    for (const auto& literal : problem.get_goal_condition<StaticTag>())
    {
        if (literal->get_polarity() != initial.positive_static_initial_atoms_bitset.get(literal->get_atom()->get_index()))
        {
            m_static_goal_holds = false;
        }
    }

    boost::hana::for_each(problem.get_hana_goal_condition(),
                          [this](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);

                              boost::hana::at_key(negative_goal_atoms, key) = filter_ground_atoms(value, false);
                              boost::hana::at_key(positive_goal_atoms, key) = filter_ground_atoms(value, true);

                              for (const auto& literal : value)
                              {
                                  if (literal->get_polarity())
                                  {
                                      boost::hana::at_key(positive_goal_atoms_bitset, key).set(literal->get_atom()->get_index());
                                  }
                                  else
                                  {
                                      boost::hana::at_key(negative_goal_atoms_bitset, key).set(literal->get_atom()->get_index());
                                  }
                              }

                              for (const auto& atom_index : boost::hana::at_key(negative_goal_atoms_bitset, key))
                              {
                                  boost::hana::at_key(negative_goal_atoms_indices, key).push_back(atom_index);
                              }
                              for (const auto& atom_index : boost::hana::at_key(positive_goal_atoms_bitset, key))
                              {
                                  boost::hana::at_key(positive_goal_atoms_indices, key).push_back(atom_index);
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

problem::GroundingDetails::GroundingDetails(const ProblemImpl& problem) :
    parent(&problem),
    action_infos(std::nullopt),
    per_action_data(problem.get_domain()->get_actions().size()),
    per_axiom_data(problem.get_problem_and_domain_axioms().size())
{
}

problem::Details::Details() : parent(nullptr) {}

problem::Details::Details(const ProblemImpl& problem) : parent(&problem), initial(problem), goal(problem, initial), axiom(problem), grounding(problem) {}

/* Printing */
std::ostream& operator<<(std::ostream& out, const ProblemImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Problem element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

}
