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
    m_setadd_action_annotations.resize(this->m_unary_actions.size());
    m_setadd_axiom_annotations.resize(this->m_unary_axioms.size());
    m_setadd_proposition_annotations.resize(this->m_propositions.size());
}

SetAddHeuristic SetAddHeuristicImpl::create(const DeleteRelaxedProblemExplorator& delete_relaxation)
{
    return std::make_shared<SetAddHeuristicImpl>(delete_relaxation);
}

void SetAddHeuristicImpl::initialize_and_annotations_impl(const Action& action)
{
    auto& annotation = this->m_action_annotations[action.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = action.get_num_preconditions();
    auto& setadd_annotation = m_setadd_action_annotations[action.get_index()];
    get_achievers(setadd_annotation).clear();
}

void SetAddHeuristicImpl::initialize_and_annotations_impl(const Axiom& axiom)
{
    auto& annotation = this->m_axiom_annotations[axiom.get_index()];
    get_cost(annotation) = 0;
    get_num_unsatisfied_preconditions(annotation) = axiom.get_num_preconditions();
    auto& setadd_annotation = m_setadd_axiom_annotations[axiom.get_index()];
    get_achievers(setadd_annotation).clear();
}

void SetAddHeuristicImpl::initialize_or_annotations_impl(const Proposition& proposition)
{
    auto& annotation = this->m_proposition_annotations[proposition.get_index()];
    get_cost(annotation) = MAX_DISCRETE_COST;
    auto& setadd_annotation = m_setadd_proposition_annotations[proposition.get_index()];
    get_achievers(setadd_annotation).clear();
}

void SetAddHeuristicImpl::initialize_or_annotations_and_queue_impl(const Proposition& proposition)
{
    auto& annotation = this->m_proposition_annotations[proposition.get_index()];
    get_cost(annotation) = 0;
    this->m_queue.insert(0, QueueEntry { proposition.get_index(), 0 });
}

void SetAddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Action& action)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& action_annotation = this->m_action_annotations[action.get_index()];
    auto& setadd_proposition_annotation = m_setadd_proposition_annotations[proposition.get_index()];
    auto& setadd_action_annotation = m_setadd_action_annotations[action.get_index()];

    get_cost(action_annotation) = std::max(get_cost(proposition_annotation), get_cost(action_annotation));
    get_achievers(setadd_action_annotation).insert(get_achievers(setadd_proposition_annotation).begin(), get_achievers(setadd_proposition_annotation).end());
}

void SetAddHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Axiom& axiom)
{
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
    auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];
    auto& setadd_proposition_annotation = m_setadd_proposition_annotations[proposition.get_index()];
    auto& setadd_axiom_annotation = m_setadd_axiom_annotations[axiom.get_index()];

    get_cost(axiom_annotation) = get_cost(proposition_annotation);
    get_achievers(setadd_axiom_annotation).insert(get_achievers(setadd_proposition_annotation).begin(), get_achievers(setadd_proposition_annotation).end());
}

void SetAddHeuristicImpl::update_or_annotation_impl(const Action& action, const Proposition& proposition)
{
    const auto& action_annotation = this->m_action_annotations[action.get_index()];
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

    const auto firing_cost = get_cost(action_annotation) + 1;

    if (firing_cost < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = firing_cost;

        auto& setadd_proposition_annotation = m_setadd_proposition_annotations[proposition.get_index()];
        auto& setadd_action_annotation = m_setadd_action_annotations[action.get_index()];
        get_achievers(setadd_proposition_annotation) = get_achievers(setadd_action_annotation);
        get_achievers(setadd_proposition_annotation).insert(action.get_index());

        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

void SetAddHeuristicImpl::update_or_annotation_impl(const Axiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];
    auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

    if (get_cost(axiom_annotation) < get_cost(proposition_annotation))
    {
        get_cost(proposition_annotation) = get_cost(axiom_annotation);

        auto& setadd_proposition_annotation = m_setadd_proposition_annotations[proposition.get_index()];
        auto& setadd_axiom_annotation = m_setadd_axiom_annotations[axiom.get_index()];
        get_achievers(setadd_proposition_annotation) = get_achievers(setadd_axiom_annotation);

        this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
    }
}

DiscreteCost SetAddHeuristicImpl::extract_impl()
{
    // Ensure that this function is called only if the goal is satisfied in the relaxed exploration.
    assert(this->m_num_unsat_goals == 0);

    m_total_goal_annotations.clear();

    for (const auto proposition_index : this->m_goal_propositions)
    {
        const auto& annotation = this->m_setadd_proposition_annotations[proposition_index];
        m_total_goal_annotations.insert(get_achievers(annotation).begin(), get_achievers(annotation).end());
    }

    std::cout << "Total cost: " << m_total_goal_annotations.size() << std::endl;

    return m_total_goal_annotations.size();
}

}
