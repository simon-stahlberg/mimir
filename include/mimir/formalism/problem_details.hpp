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

#ifndef MIMIR_FORMALISM_PROBLEM_DETAILS_HPP_
#define MIMIR_FORMALISM_PROBLEM_DETAILS_HPP_

#include "mimir/common/declarations.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/axiom_partitioning.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/grounding_table.hpp"

namespace mimir::formalism::problem
{

struct ObjectDetails
{
    const ProblemImpl* parent;

    ToObjectMap<std::string> name_to_object;
    ToObjectMap<std::string> name_to_problem_or_domain_object;

    ObjectDetails();
    ObjectDetails(const ProblemImpl& problem);
};

/**
 * Predicate
 */

struct PredicateDetails
{
    const ProblemImpl* parent;

    ToPredicateMap<std::string, DerivedTag> name_to_derived_predicate;
    ToPredicateMap<std::string, DerivedTag> name_to_problem_or_domain_derived_predicate;

    PredicateDetails();
    PredicateDetails(const ProblemImpl& problem);
};

/**
 * Initial
 */

struct InitialDetails
{
    const ProblemImpl* parent;

    GroundAtomList<StaticTag> positive_static_initial_atoms;
    FlatBitset positive_static_initial_atoms_bitset;
    FlatIndexList positive_static_initial_atoms_indices;
    GroundAtomList<FluentTag> positive_fluent_initial_atoms;

    GroundFunctionList<StaticTag> static_initial_functions;
    FlatDoubleLists<StaticTag, FluentTag> initial_function_to_value;

    InitialDetails();
    InitialDetails(const ProblemImpl& problem);
};

/**
 * Goal
 */

struct GoalDetails
{
    const ProblemImpl* parent;

    bool m_static_goal_holds;

    HanaContainer<GroundAtomLists<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> goal_atoms;
    HanaContainer<FlatBitsets<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> goal_atoms_bitset;
    HanaContainer<FlatIndexLists<StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> goal_atoms_indices;
    GroundConjunctiveCondition condition;

    GoalDetails();
    GoalDetails(ProblemImpl& problem, const InitialDetails& initial);
};

/**
 * Axiom
 */

struct AxiomDetails
{
    const ProblemImpl* parent;

    std::vector<AxiomPartition> problem_and_domain_axiom_partitioning;  ///< Obtained from stratification

    AxiomDetails();
    AxiomDetails(const ProblemImpl& problem);
};

/**
 * Grounding
 */

using CandidateParameterObjectBinding = IndexList;
using CandidateParameterObjectBindingList = std::vector<CandidateParameterObjectBinding>;

struct ConditionalEffectGroundingInfo
{
    explicit ConditionalEffectGroundingInfo(CandidateParameterObjectBindingList candidate_variable_bindings) :
        candidate_variable_bindings(std::move(candidate_variable_bindings))
    {
    }

    CandidateParameterObjectBindingList candidate_variable_bindings;
};

using ConditionalEffectGroundingInfoList = std::vector<ConditionalEffectGroundingInfo>;

struct ActionGroundingInfo
{
    explicit ActionGroundingInfo(ConditionalEffectGroundingInfoList conditional_effect_infos) : conditional_effect_infos(std::move(conditional_effect_infos)) {}

    ConditionalEffectGroundingInfoList conditional_effect_infos;
};

using ActionGroundingInfoList = std::vector<ActionGroundingInfo>;

struct GroundingDetails
{
    const ProblemImpl* parent;

    mutable std::optional<ActionGroundingInfoList> action_infos;  ///< lazyly initialized
    const ActionGroundingInfoList& get_action_infos() const;      ///< lazily initializes problem-specific action infos.

    using PDDLTypeToGroundingTable = boost::hana::map<boost::hana::pair<boost::hana::type<GroundAction>, GroundingTableList<GroundAction>>,
                                                      boost::hana::pair<boost::hana::type<GroundAxiom>, GroundingTableList<GroundAxiom>>>;

    PDDLTypeToGroundingTable grounding_tables;

    /* For ground actions and axioms we also create a reusable builder. */

    GroundingDetails();
    GroundingDetails(const ProblemImpl& problem);
};

struct Details
{
    const ProblemImpl* parent;

    ObjectDetails objects;
    PredicateDetails predicates;
    InitialDetails initial;
    GoalDetails goal;
    AxiomDetails axiom;
    GroundingDetails grounding;

    Details();
    Details(ProblemImpl& problem);
};
}

#endif
