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

#include "mimir/search/heuristics/max.hpp"

namespace mimir::search
{
using namespace rpg;

/**
 * HMax
 */

MaxHeuristicImpl::MaxHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : RelaxedPlanningGraph<MaxHeuristicImpl>(delete_relaxation) {}

MaxHeuristic MaxHeuristicImpl::create(const DeleteRelaxedProblemExplorator& delete_relaxation) { return std::make_shared<MaxHeuristicImpl>(delete_relaxation); }

void MaxHeuristicImpl::initialize_and_annotations_impl(const UnaryGroundAction& action)
{
    auto& annotation = this->m_action_annotations[action.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = action.get_num_preconditions();
}

void MaxHeuristicImpl::initialize_and_annotations_impl(const UnaryGroundAxiom& axiom)
{
    auto& annotation = this->m_axiom_annotations[axiom.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = axiom.get_num_preconditions();
}

void MaxHeuristicImpl::initialize_or_annotations_impl(const Proposition& proposition)
{
    auto& annotation = this->m_proposition_annotations[proposition.get_index()];
    get_cost(annotation) = MAX_DISCRETE_COST;
}

void MaxHeuristicImpl::initialize_or_annotations_and_queue_impl(const Proposition& proposition)
{
    auto& annotation = this->m_proposition_annotations[proposition.get_index()];
    get_cost(annotation) = 0;
    this->m_queue.insert(0, QueueEntry { proposition.get_index(), 0 });
}

void MaxHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& action_annotation = this->m_action_annotations[action.get_index()];

    get_cost(action_annotation) = std::max(get_cost(action_annotation), get_cost(proposition_annotation));
}

void MaxHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];

    get_cost(axiom_annotation) = std::max(get_cost(axiom_annotation), get_cost(proposition_annotation));
}

void MaxHeuristicImpl::update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition)
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

void MaxHeuristicImpl::update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

    if (get_cost(axiom_annotation) < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = get_cost(axiom_annotation);
        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

DiscreteCost MaxHeuristicImpl::extract_impl()
{
    // Ensure that this function is called only if the goal is satisfied in the relaxed exploration.
    assert(this->m_num_unsat_goals == 0);

    auto total_cost = DiscreteCost(0);
    for (const auto proposition_index : this->m_goal_propositions)
    {
        const auto& annotation = this->m_proposition_annotations[proposition_index];
        total_cost = std::max(total_cost, get_cost(annotation));
    }
    std::cout << "Total cost: " << total_cost << std::endl;

    return total_cost;
}
}
