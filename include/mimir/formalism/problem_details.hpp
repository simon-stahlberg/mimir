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

#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/axiom_stratification.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/grounding_table.hpp"

namespace mimir::formalism::problem
{

/**
 * Initial
 */

struct InitialDetails
{
    const ProblemImpl* parent;

    GroundAtomList<StaticTag> positive_static_initial_atoms;
    FlatBitset positive_static_initial_atoms_bitset;
    FlatIndexList positive_static_initial_atoms_indices;
    AssignmentSet<StaticTag> positive_static_initial_assignment_set;
    NumericAssignmentSet<StaticTag> static_initial_numeric_assignment_set;

    GroundAtomList<FluentTag> positive_fluent_initial_atoms;

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

    GroundAtomLists<StaticTag, FluentTag, DerivedTag> positive_goal_atoms;
    FlatBitsets<StaticTag, FluentTag, DerivedTag> positive_goal_atoms_bitset;
    FlatIndexLists<StaticTag, FluentTag, DerivedTag> positive_goal_atoms_indices;

    GroundAtomLists<StaticTag, FluentTag, DerivedTag> negative_goal_atoms;
    FlatBitsets<StaticTag, FluentTag, DerivedTag> negative_goal_atoms_bitset;
    FlatIndexLists<StaticTag, FluentTag, DerivedTag> negative_goal_atoms_indices;

    GoalDetails();
    GoalDetails(const ProblemImpl& problem, const InitialDetails& initial);
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
    explicit ConditionalEffectGroundingInfo(CandidateParameterObjectBindingList candidate_variable_bindings) : candidate_variable_bindings(std::move(candidate_variable_bindings)){}

    CandidateParameterObjectBindingList candidate_variable_bindings;
};

using ConditionalEffectGroundingInfoList = std::vector<ConditionalEffectGroundingInfo>;

struct ActionGroundingInfo
{
    explicit ActionGroundingInfo(ConditionalEffectGroundingInfoList conditional_effect_infos) : conditional_effect_infos(std::move(conditional_effect_infos)){}

    ConditionalEffectGroundingInfoList conditional_effect_infos;
};

using ActionGroundingInfoList = std::vector<ActionGroundingInfo>;

struct GroundingDetails
{
    const ProblemImpl* parent;

    mutable std::optional<ActionGroundingInfoList> action_infos;  ///< lazyly initialized
    const ActionGroundingInfoList& get_action_infos() const;      ///< lazily initializes problem-specific action infos.

    // A table for each pair (polarity,predicate_index) since those are context independent.

    template<typename T>
    using LiteralGroundingTableList = std::array<std::vector<GroundingTable<T>>, 2>;

    using PDDLTypeToGroundingTable =
        boost::hana::map<boost::hana::pair<boost::hana::type<GroundLiteral<StaticTag>>, LiteralGroundingTableList<GroundLiteral<StaticTag>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<FluentTag>>, LiteralGroundingTableList<GroundLiteral<FluentTag>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<DerivedTag>>, LiteralGroundingTableList<GroundLiteral<DerivedTag>>>,
                         boost::hana::pair<boost::hana::type<GroundFunction<StaticTag>>, GroundingTableList<GroundFunction<StaticTag>>>,
                         boost::hana::pair<boost::hana::type<GroundFunction<FluentTag>>, GroundingTableList<GroundFunction<FluentTag>>>,
                         boost::hana::pair<boost::hana::type<GroundFunction<AuxiliaryTag>>, GroundingTableList<GroundFunction<AuxiliaryTag>>>,
                         boost::hana::pair<boost::hana::type<GroundFunctionExpression>, GroundingTableList<GroundFunctionExpression>>>;

    PDDLTypeToGroundingTable grounding_tables;

    /* For ground actions and axioms we also create a reusable builder. */

    GroundActionImplSet ground_actions;
    GroundActionList ground_actions_by_index;
    std::vector<std::pair<GroundActionImpl, GroundingTable<GroundAction>>> per_action_data;

    GroundAxiomImplSet ground_axioms;
    GroundAxiomList ground_axioms_by_index;
    std::vector<std::pair<GroundAxiomImpl, GroundingTable<GroundAxiom>>> per_axiom_data;

    GroundingDetails();
    GroundingDetails(const ProblemImpl& problem);
};

struct Details
{
    const ProblemImpl* parent;

    InitialDetails initial;
    GoalDetails goal;
    AxiomDetails axiom;
    GroundingDetails grounding;

    Details();
    Details(const ProblemImpl& problem);
};
}

#endif
