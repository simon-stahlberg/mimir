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

void AddHeuristicImpl::initialize_and_annotations_impl(const Action& action)
{
    auto& annotation = this->get_structures_annotations<Action>()[action.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = action.get_num_preconditions();
}

void AddHeuristicImpl::initialize_and_annotations_impl(const Axiom& axiom)
{
    auto& annotation = this->get_structures_annotations<Axiom>()[axiom.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = axiom.get_num_preconditions();
}

void AddHeuristicImpl::initialize_or_annotations_impl(const Proposition& proposition)
{
    auto& annotation = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotation) = MAX_DISCRETE_COST;
}

void AddHeuristicImpl::initialize_or_annotations_and_queue_impl(const Proposition& proposition)
{
    auto& annotation = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotation) = 0;
    this->m_queue.insert(0, QueueEntry { proposition.get_index(), 0 });
}

void AddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Action& action)
{
    auto& proposition_annotation = this->get_proposition_annotations()[proposition.get_index()];
    auto& action_annotation = this->get_structures_annotations<Action>()[action.get_index()];

    get_cost(action_annotation) += get_cost(proposition_annotation);
}

void AddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Axiom& axiom)
{
    auto& proposition_annotation = this->get_proposition_annotations()[proposition.get_index()];
    auto& axiom_annotation = this->get_structures_annotations<Axiom>()[axiom.get_index()];

    get_cost(axiom_annotation) = std::max(get_cost(proposition_annotation), get_cost(axiom_annotation));
}

void AddHeuristicImpl::update_or_annotation_impl(const Action& action, const Proposition& proposition)
{
    const auto& action_annotation = this->get_structures_annotations<Action>()[action.get_index()];
    auto& proposition_annotation = this->get_proposition_annotations()[proposition.get_index()];

    const auto firing_cost = get_cost(action_annotation) + 1;

    if (firing_cost < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = firing_cost;
        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

void AddHeuristicImpl::update_or_annotation_impl(const Axiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotation = this->get_structures_annotations<Axiom>()[axiom.get_index()];
    auto& proposition_annotation = this->get_proposition_annotations()[proposition.get_index()];

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
    for (const auto proposition_index : this->get_goal_propositions())
    {
        const auto& annotation = this->get_proposition_annotations()[proposition_index];
        total_cost += get_cost(annotation);
    }

    std::cout << "Total cost: " << total_cost << std::endl;

    return total_cost;
}

}
