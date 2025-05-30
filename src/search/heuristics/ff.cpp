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

#include "mimir/search/heuristics/ff.hpp"

#include "mimir/search/applicability.hpp"

namespace mimir::search
{
using namespace rpg;

/**
 * HMax
 */

FFHeuristicImpl::FFHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : RelaxedPlanningGraph<FFHeuristicImpl>(delete_relaxation)
{
    get<Action>(get_ff_structures_annotations()).resize(get<Action>(this->get_structures()).size());
    get<Axiom>(get_ff_structures_annotations()).resize(get<Axiom>(this->get_structures()).size());
    get_ff_proposition_annotations().resize(this->get_propositions().size());
}

FFHeuristic FFHeuristicImpl::create(const DeleteRelaxedProblemExplorator& delete_relaxation) { return std::make_shared<FFHeuristicImpl>(delete_relaxation); }

void FFHeuristicImpl::initialize_and_annotations_impl(const Action& action)
{
    auto& annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    get_cost(annotations) = 0;
    get_num_unsatisfied_preconditions(annotations) = action.get_num_preconditions();
    auto& ff_annotations = get<Action>(get_ff_structures_annotations())[action.get_index()];
    get_achiever(ff_annotations) = MAX_INDEX;
}

void FFHeuristicImpl::initialize_and_annotations_impl(const Axiom& axiom)
{
    auto& annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    get_cost(annotations) = 0;
    get_num_unsatisfied_preconditions(annotations) = axiom.get_num_preconditions();
    auto& ff_annotations = get<Axiom>(get_ff_structures_annotations())[axiom.get_index()];
    get_achiever(ff_annotations) = MAX_INDEX;
}

void FFHeuristicImpl::initialize_or_annotations_impl(const Proposition& proposition)
{
    auto& annotations = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotations) = MAX_DISCRETE_COST;
    auto& ff_annotations = get_ff_proposition_annotations()[proposition.get_index()];
    get_achiever(ff_annotations) = MAX_INDEX;
    is_marked(ff_annotations) = false;
}

void FFHeuristicImpl::initialize_or_annotations_and_queue_impl(const Proposition& proposition)
{
    auto& annotations = this->get_proposition_annotations()[proposition.get_index()];
    get_cost(annotations) = 0;
    this->m_queue.insert(0, QueueEntry { proposition.get_index(), 0 });
}

void FFHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Action& action)
{
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];
    auto& action_annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    auto& ff_action_annotations = get<Action>(get_ff_structures_annotations())[action.get_index()];

    get_cost(action_annotations) = std::max(get_cost(proposition_annotations), get_cost(action_annotations));
    get_achiever(ff_action_annotations) = proposition.get_index();
}

void FFHeuristicImpl::update_and_annotation_impl(const Proposition& proposition, const Axiom& axiom)
{
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];
    auto& axiom_annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    auto& ff_proposition_annotations = get_ff_proposition_annotations()[proposition.get_index()];
    auto& ff_axiom_annotations = get<Axiom>(get_ff_structures_annotations())[axiom.get_index()];

    get_cost(axiom_annotations) = get_cost(proposition_annotations);
    get_achiever(ff_axiom_annotations) = get_achiever(ff_proposition_annotations);  // Forward the achiever action
}

void FFHeuristicImpl::update_or_annotation_impl(const Action& action, const Proposition& proposition)
{
    const auto& action_annotations = get<Action>(this->get_structures_annotations())[action.get_index()];
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];

    const auto firing_cost = get_cost(action_annotations) + 1;

    if (firing_cost < get_cost(proposition_annotations))
    {
        get_cost(proposition_annotations) = firing_cost;

        auto& ff_proposition_annotations = get_ff_proposition_annotations()[proposition.get_index()];
        get_achiever(ff_proposition_annotations) = action.get_index();

        this->m_queue.insert(get_cost(proposition_annotations), QueueEntry { proposition.get_index(), get_cost(proposition_annotations) });
    }
}

void FFHeuristicImpl::update_or_annotation_impl(const Axiom& axiom, const Proposition& proposition)
{
    const auto& axiom_annotations = get<Axiom>(this->get_structures_annotations())[axiom.get_index()];
    auto& proposition_annotations = this->get_proposition_annotations()[proposition.get_index()];

    if (get_cost(axiom_annotations) < get_cost(proposition_annotations))
    {
        get_cost(proposition_annotations) = get_cost(axiom_annotations);

        auto& ff_proposition_annotations = get_ff_proposition_annotations()[proposition.get_index()];
        auto& ff_axiom_annotations = get<Axiom>(get_ff_structures_annotations())[axiom.get_index()];
        get_achiever(ff_proposition_annotations) = get_achiever(ff_axiom_annotations);  // Forward the achiever action

        this->m_queue.insert(get_cost(proposition_annotations), QueueEntry { proposition.get_index(), get_cost(proposition_annotations) });
    }
}

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Action& action)
{
    for (const auto& atom_index : action.template get_preconditions<R, P>()->compressed_range())
    {
        extract_relaxed_plan_and_preferred_operators_recursively(state, this->get_propositions()[get<R, P>(this->get_offsets())[atom_index]]);
    }
    for (const auto& atom_index : action.template get_conditional_preconditions<R, P>()->compressed_range())
    {
        extract_relaxed_plan_and_preferred_operators_recursively(state, this->get_propositions()[get<R, P>(this->get_offsets())[atom_index]]);
    }
}

template void
FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::FluentTag>(State state, const rpg::Action& action);
template void
FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::DerivedTag>(State state,
                                                                                                                         const rpg::Action& action);
template void
FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::FluentTag>(State state, const rpg::Action& action);
template void
FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::DerivedTag>(State state,
                                                                                                                         const rpg::Action& action);

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Axiom& axiom)
{
    for (const auto& atom_index : axiom.template get_preconditions<R, P>()->compressed_range())
    {
        extract_relaxed_plan_and_preferred_operators_recursively(state, this->get_propositions()[get<R, P>(this->get_offsets())[atom_index]]);
    }
}

template void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::FluentTag>(State state,
                                                                                                                                      const rpg::Axiom& axiom);
template void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::DerivedTag>(State state,
                                                                                                                                       const rpg::Axiom& axiom);
template void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::FluentTag>(State state,
                                                                                                                                      const rpg::Axiom& axiom);
template void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::DerivedTag>(State state,
                                                                                                                                       const rpg::Axiom& axiom);

void FFHeuristicImpl::extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Proposition& proposition)
{
    auto& ff_proposition_annotations = get_ff_proposition_annotations()[proposition.get_index()];

    if (is_marked(ff_proposition_annotations))
        return;
    is_marked(ff_proposition_annotations) = true;

    if (get_achiever(ff_proposition_annotations) == MAX_INDEX)
        return;

    const auto& action = get<Action>(this->get_structures())[get_achiever(ff_proposition_annotations)];

    extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::FluentTag>(state, action);
    extract_relaxed_plan_and_preferred_operators_recursively<formalism::PositiveTag, formalism::DerivedTag>(state, action);
    extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::FluentTag>(state, action);
    extract_relaxed_plan_and_preferred_operators_recursively<formalism::NegativeTag, formalism::DerivedTag>(state, action);

    m_relaxed_plan.insert(action.get_unrelaxed_action());

    if (is_applicable(action.get_unrelaxed_action(), this->get_problem(), state))
    {
        this->m_preferred_actions.data.insert(action.get_unrelaxed_action());
    }
}

DiscreteCost FFHeuristicImpl::extract_impl(State state)
{
    // Ensure that this function is called only if the goal is satisfied in the relaxed exploration.
    assert(this->m_num_unsat_goals == 0);

    get_relaxed_plan().clear();
    this->m_preferred_actions.data.clear();

    for (const auto proposition_index : this->get_goal_propositions())
    {
        extract_relaxed_plan_and_preferred_operators_recursively(state, this->get_propositions()[proposition_index]);
    }

    // std::cout << "Total cost: " << get_relaxed_plan().size() << std::endl;
    // std::cout << "Num preferred actions: " << get_preferred_actions().size() << std::endl;

    return get_relaxed_plan().size();
}

}
