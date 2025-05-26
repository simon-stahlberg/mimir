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

#include "mimir/search/heuristics/add.hpp"

namespace mimir::search
{
using namespace rpg;

/**
 * HMax
 */

AddHeuristicImpl::AddHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : RelaxedPlanningGraph<AddHeuristicImpl>(delete_relaxation) {}

AddHeuristic AddHeuristicImpl::create(const DeleteRelaxedProblemExplorator& delete_relaxation) { return std::make_shared<AddHeuristicImpl>(delete_relaxation); }

void AddHeuristicImpl::initialize_and_annotations_impl()
{
    this->m_action_annotations.resize(this->m_unary_actions.size());
    for (size_t i = 0; i < this->m_unary_actions.size(); ++i)
    {
        auto& annotation = this->m_action_annotations[i];
        const auto& action = this->m_unary_actions[i];
        get_cost(annotation) = 0;
        get_num_unsatisfied_preconditions(annotation) = action.get_num_preconditions();
    }
    this->m_axiom_annotations.resize(this->m_unary_axioms.size());
    for (size_t i = 0; i < this->m_unary_axioms.size(); ++i)
    {
        auto& annotation = this->m_axiom_annotations[i];
        const auto& axiom = this->m_unary_axioms[i];
        get_cost(annotation) = 0;
        get_num_unsatisfied_preconditions(annotation) = axiom.get_num_preconditions();
    }
}

void AddHeuristicImpl::initialize_or_annotations_impl(State state)
{
    this->m_proposition_annotations.resize(this->m_propositions.size());
    for (size_t i = 0; i < this->m_propositions.size(); ++i)
    {
        auto& annotation = this->m_proposition_annotations[i];
        get_cost(annotation) = MAX_DISCRETE_COST;
    }

    this->m_queue.clear();

    for (const auto atom_index : state->get_atoms<formalism::FluentTag>())
    {
        const auto proposition_index = this->m_fluent_offsets[atom_index];
        auto& annotation = this->m_proposition_annotations[proposition_index];
        get_cost(annotation) = 0;
        this->m_queue.insert(0, QueueEntry { proposition_index, 0 });
    }
    for (const auto atom_index : state->get_atoms<formalism::DerivedTag>())
    {
        const auto proposition_index = this->m_derived_offsets[atom_index];
        auto& annotation = this->m_proposition_annotations[proposition_index];
        get_cost(annotation) = 0;
        this->m_queue.insert(0, QueueEntry { proposition_index, 0 });
    }
    // Trivial dummy proposition to trigger actions and axioms without preconditions
    auto& annotation = this->m_proposition_annotations[0];
    get_cost(annotation) = 0;
    this->m_queue.insert(0, QueueEntry { 0, 0 });
}

void AddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& action_annotation = this->m_action_annotations[action.get_index()];

    get_cost(action_annotation) += get_cost(proposition_annotation);
}

void AddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];

    get_cost(axiom_annotation) = get_cost(proposition_annotation);
}

void AddHeuristicImpl::update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition)
{
    const auto& action_annotation = this->m_action_annotations[action.get_index()];
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

    const auto cost = get_cost(action_annotation) + 1;

    if (cost < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = cost;
        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

void AddHeuristicImpl::update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

    if (get_cost(axiom_annotation) < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = get_cost(axiom_annotation);
        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

DiscreteCost AddHeuristicImpl::extract_impl()
{
    // Ensure that this function is called only if the goal is satisfied in the relaxed exploration.
    assert(this->m_num_unsat_goals == 0);

    auto total_cost = DiscreteCost(0);
    for (const auto proposition_index : this->m_goal_propositions)
    {
        const auto& annotation = this->m_proposition_annotations[proposition_index];
        total_cost += get_cost(annotation);
    }
    return total_cost;
}

}
