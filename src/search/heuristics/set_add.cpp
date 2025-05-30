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

#include "mimir/search/heuristics/set_add.hpp"

namespace mimir::search
{
using namespace rpg;

/**
 * HMax
 */

SetAddHeuristicImpl::SetAddHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : RelaxedPlanningGraph<SetAddHeuristicImpl>(delete_relaxation)
{
    get_setadd_structure_annotations<Action>().resize(get<Action>(this->get_structures()).size());
    get_setadd_structure_annotations<Axiom>().resize(get<Axiom>(this->get_structures()).size());
    get_setadd_proposition_annotations().resize(this->get_propositions().size());
}

SetAddHeuristic SetAddHeuristicImpl::create(const DeleteRelaxedProblemExplorator& delete_relaxation)
{
    return std::make_shared<SetAddHeuristicImpl>(delete_relaxation);
}

void SetAddHeuristicImpl::initialize_and_annotations_impl(const Action& action)
{
    auto& annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    get_cost(annotations) = 0;
    get_num_unsatisfied_preconditions(annotations) = action.get_num_preconditions();
    auto& setadd_annotations = get_setadd_structure_annotations<Action>()[action.get_index()];
    get_achievers(setadd_annotations).clear();
}

void SetAddHeuristicImpl::initialize_and_annotations_impl(const Axiom& axiom)
{
    auto& annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    get_cost(annotations) = 0;
    get_num_unsatisfied_preconditions(annotations) = axiom.get_num_preconditions();
    auto& setadd_annotations = get_setadd_structure_annotations<Axiom>()[axiom.get_index()];
    get_achievers(setadd_annotations).clear();
}

void SetAddHeuristicImpl::initialize_or_annotations_impl(const Proposition& proposition)
{
    auto& annotations = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotations) = MAX_DISCRETE_COST;
    auto& setadd_annotations = get_setadd_proposition_annotations()[proposition.get_index()];
    get_achievers(setadd_annotations).clear();
}

void SetAddHeuristicImpl::initialize_or_annotations_and_queue_impl(const Proposition& proposition)
{
    auto& annotations = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotations) = 0;
    this->m_queue.insert(0, QueueEntry { proposition.get_index(), 0 });
}

void SetAddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Action& action)
{
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];
    auto& action_annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    auto& setadd_proposition_annotations = get_setadd_proposition_annotations()[proposition.get_index()];
    auto& setadd_action_annotations = get_setadd_structure_annotations<Action>()[action.get_index()];

    get_cost(action_annotations) = std::max(get_cost(proposition_annotations), get_cost(action_annotations));
    get_achievers(setadd_action_annotations).insert(get_achievers(setadd_proposition_annotations).begin(), get_achievers(setadd_proposition_annotations).end());
}

void SetAddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Axiom& axiom)
{
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];
    auto& axiom_annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    auto& setadd_proposition_annotations = get_setadd_proposition_annotations()[proposition.get_index()];
    auto& setadd_axiom_annotations = get_setadd_structure_annotations<Axiom>()[axiom.get_index()];

    get_cost(axiom_annotations) = get_cost(proposition_annotations);
    get_achievers(setadd_axiom_annotations).insert(get_achievers(setadd_proposition_annotations).begin(), get_achievers(setadd_proposition_annotations).end());
}

void SetAddHeuristicImpl::update_or_annotation_impl(const Action& action, const Proposition& proposition)
{
    const auto& action_annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];

    const auto firing_cost = get_cost(action_annotations) + 1;

    if (firing_cost < get_cost(proposition_annotations))
    {
        get_cost(proposition_annotations) = firing_cost;

        auto& setadd_proposition_annotations = get_setadd_proposition_annotations()[proposition.get_index()];
        auto& setadd_action_annotations = get_setadd_structure_annotations<Action>()[action.get_index()];
        get_achievers(setadd_proposition_annotations) = get_achievers(setadd_action_annotations);
        get_achievers(setadd_proposition_annotations).insert(action.get_index());

        this->m_queue.insert(get_cost(proposition_annotations), QueueEntry { proposition.get_index(), get_cost(proposition_annotations) });
    }
}

void SetAddHeuristicImpl::update_or_annotation_impl(const Axiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];

    if (get_cost(axiom_annotations) < get_cost(proposition_annotations))
    {
        get_cost(proposition_annotations) = get_cost(axiom_annotations);

        auto& setadd_proposition_annotations = get_setadd_proposition_annotations()[proposition.get_index()];
        auto& setadd_axiom_annotations = get_setadd_structure_annotations<Axiom>()[axiom.get_index()];
        get_achievers(setadd_proposition_annotations) = get_achievers(setadd_axiom_annotations);

        this->m_queue.insert(get_cost(proposition_annotations), QueueEntry { proposition.get_index(), get_cost(proposition_annotations) });
    }
}

DiscreteCost SetAddHeuristicImpl::extract_impl(State)
{
    // Ensure that this function is called only if the goal is satisfied in the relaxed exploration.
    assert(this->m_num_unsat_goals == 0);

    get_total_goal_annotations().clear();

    for (const auto proposition_index : this->get_goal_propositions())
    {
        const auto& annotations = get_setadd_proposition_annotations()[proposition_index];
        get_total_goal_annotations().insert(get_achievers(annotations).begin(), get_achievers(annotations).end());
    }

    // std::cout << "Total cost: " << get_total_goal_annotations().size() << std::endl;

    return get_total_goal_annotations().size();
}

}
