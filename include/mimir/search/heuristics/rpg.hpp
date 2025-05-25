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

#ifndef MIMIR_SEARCH_HEURISTICS_RPG_HPP_
#define MIMIR_SEARCH_HEURISTICS_RPG_HPP_

#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"

#include <deque>

namespace mimir::search::rpg
{
class UnaryGroundAction
{
public:
    UnaryGroundAction(Index index,
                      const FlatIndexList& fluent_preconditions,
                      const FlatIndexList& derived_preconditions,
                      const FlatIndexList& cond_fluent_preconditions,
                      const FlatIndexList& cond_derived_preconditions,
                      Index fluent_effect) :
        m_index(index),
        m_fluent_preconditions(fluent_preconditions),
        m_derived_preconditions(derived_preconditions),
        m_cond_fluent_preconditions(cond_fluent_preconditions),
        m_cond_derived_preconditions(cond_derived_preconditions),
        m_fluent_effect(fluent_effect)
    {
    }

    Index get_index() const { return m_index; }
    const FlatIndexList& get_fluent_preconditions() const { return m_fluent_preconditions; }
    const FlatIndexList& get_derived_preconditions() const { return m_derived_preconditions; }
    const FlatIndexList& get_cond_fluent_preconditions() const { return m_cond_fluent_preconditions; }
    const FlatIndexList& get_cond_derived_preconditions() const { return m_cond_derived_preconditions; }
    Index get_fluent_effect() const { return m_fluent_effect; }

private:
    Index m_index;
    const FlatIndexList& m_fluent_preconditions;
    const FlatIndexList& m_derived_preconditions;
    const FlatIndexList& m_cond_fluent_preconditions;
    const FlatIndexList& m_cond_derived_preconditions;
    Index m_fluent_effect;
};
using UnaryGroundActionList = std::vector<UnaryGroundAction>;

struct UnaryGroundActionAnnotations
{
    size_t num_unsatisfied_preconditions;
    DiscreteCost cost;
};
using UnaryGroundActionAnnotationsList = std::vector<UnaryGroundActionAnnotations>;

class UnaryGroundAxiom
{
public:
    UnaryGroundAxiom(Index index, const FlatIndexList& fluent_preconditions, const FlatIndexList& derived_preconditions, Index derived_effect) :
        m_index(index),
        m_fluent_preconditions(fluent_preconditions),
        m_derived_preconditions(derived_preconditions),
        m_derived_effect(derived_effect)
    {
    }

    Index get_index() const { return m_index; }
    const FlatIndexList& get_fluent_precondition() const { return m_fluent_preconditions; }
    const FlatIndexList& get_derived_precondition() const { return m_derived_preconditions; }
    Index get_fluent_effect() const { return m_derived_effect; }

private:
    Index m_index;
    const FlatIndexList& m_fluent_preconditions;
    const FlatIndexList& m_derived_preconditions;
    Index m_derived_effect;
};
using UnaryGroundAxiomList = std::vector<UnaryGroundAxiom>;

struct UnaryGroundAxiomAnnotations
{
    size_t num_unsatisfied_preconditions;
};
using UnaryGroundAxiomAnnotationsList = std::vector<UnaryGroundAxiomAnnotations>;

class Proposition
{
public:
    Proposition(Index index, IndexList is_precondition_of_actions, IndexList is_precondition_of_axioms, bool is_goal) :
        m_index(index),
        m_is_precondition_of_actions(std::move(is_precondition_of_actions)),
        m_is_precondition_of_axioms(std::move(is_precondition_of_axioms)),
        m_is_goal(is_goal)
    {
    }

    Index get_index() const { return m_index; }
    const IndexList& is_precondition_of_actions() const { return m_is_precondition_of_actions; }
    const IndexList& is_precondition_of_axioms() const { return m_is_precondition_of_axioms; }
    bool is_goal() const { return m_is_goal; }

private:
    Index m_index;
    IndexList m_is_precondition_of_actions;
    IndexList m_is_precondition_of_axioms;
    bool m_is_goal;
};

using PropositionList = std::vector<Proposition>;

template<typename... Annotations>
struct PropositionAnnotations
{
    DiscreteCost cost = std::numeric_limits<DiscreteCost>::max();
    std::tuple<Annotations...> m_annotations;
};
template<typename... Annotations>
using PropositionAnnotationsList = std::vector<PropositionAnnotations<Annotations...>>;

template<typename Derived>
class RelaxedPlanningGraph : public IHeuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    friend class Derived;

public:
    double compute_heuristic(State state, bool is_goal_state) override
    {
        self().initialize_annotations_impl();
        brfs();
        return self().extract_impl();
    }

private:
    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) :
        m_fluent_offsets(),
        m_derived_offsets(),
        m_unary_actions(),
        m_unary_axioms()
    {
        auto is_fluent_precondition_of_action = IndexMap<IndexList> {};
        auto is_derived_precondtion_of_action = IndexMap<IndexList> {};

        for (const auto& action : delete_relaxation.create_ground_actions())
        {
            for (const auto& cond_effect : action->get_conditional_effects())
            {
                for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_positive_effects())
                {
                    const auto unary_action_index = m_unary_actions.size();

                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_positive_precondition<formalism::FluentTag>())
                    {
                        is_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_positive_precondition<formalism::DerivedTag>())
                    {
                        is_derived_precondtion_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    m_unary_actions.push_back(
                        UnaryGroundAction(unary_action_index,
                                          action->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::FluentTag>(),
                                          action->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::DerivedTag>(),
                                          cond_effect->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::FluentTag>(),
                                          cond_effect->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::DerivedTag>(),
                                          eff_atom_index));
                }
            }
        }

        auto is_fluent_precondition_of_axiom = IndexMap<IndexList> {};
        auto is_derived_precondition_of_axiom = IndexMap<IndexList> {};

        for (const auto& axiom : delete_relaxation.create_ground_axioms())
        {
            const auto unary_axiom_index = m_unary_axioms.size();

            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_positive_precondition<formalism::FluentTag>())
            {
                is_fluent_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_positive_precondition<formalism::DerivedTag>())
            {
                is_derived_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }
            m_unary_axioms.push_back(UnaryGroundAxiom(unary_axiom_index,
                                                      axiom->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::FluentTag>(),
                                                      axiom->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::DerivedTag>(),
                                                      axiom->get_literal()->get_atom()->get_index()))
        }

        for (const auto& atom : delete_relaxation.create_ground_atoms<formalism::FluentTag>())
        {
            const auto proposition_index = m_propositions.size();

            if (atom->get_index() >= m_fluent_offsets.size())
            {
                m_fluent_offsets.resize(atom->get_index() + 1, MAX_INDEX);
            }
            m_fluent_offsets[atom->get_index()] = proposition_index;

            const auto is_goal = delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::FluentTag>().get(atom->get_index());

            m_propositions.emplace_back(proposition_index,
                                        std::move(is_fluent_precondition_of_action[atom->get_index()]),
                                        std::move(is_derived_precondition_of_axiom[atom->get_index]),
                                        is_goal);

            if (is_goal)
                m_goal_propositions.push_back(proposition_index);
        }
        for (const auto& atom : delete_relaxation.create_ground_atoms<formalism::DerivedTag>())
        {
            const auto proposition_index = m_propositions.size();

            if (atom->get_index() >= m_derived_offsets.size())
            {
                m_derived_offsets.resize(atom->get_index() + 1, MAX_INDEX);
            }
            m_derived_offsets[atom->get_index()] = proposition_index;

            const auto is_goal = delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::DerivedTag>().get(atom->get_index());

            m_propositions.emplace_back(proposition_index,
                                        std::move(is_fluent_precondition_of_action[atom->get_index()]),
                                        std::move(is_derived_precondition_of_axiom[atom->get_index]),
                                        delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::DerivedTag>().get(atom->get_index()));

            if (is_goal)
                m_goal_propositions.push_back(proposition_index);
        }
    }

    struct QueueEntry
    {
        Index proposition_index;
        DiscreteCost cost;
    };

    void reset() {}

    void brfs()
    {
        auto num_unsat_goals = m_goal_propositions.size();

        while (!m_queue.empty())
        {
            const auto& entry = m_queue.front();
            m_queue.pop_front();
            const auto& proposition = m_propositions[entry.proposition_index];

            if (proposition.is_goal() && --num_unsat_goals == 0)
            {
                return;
            }

            if (self().on_expand_proposition_impl(proposition, entry.cost))
            {
                return;
            }

            for (const auto action_index : proposition.is_precondition_of_action())
            {
                const auto& action = m_unary_actions[action_index];

                self().on_process_action_impl(proposition, entry.distance, action);
            }
            for (const auto axiom_index : proposition.is_precondition_of_axiom())
            {
                const auto& axiom = m_unary_axioms[axiom_index];

                self().on_process_axiom_impl(proposition, entry.distance, axiom);
            }
        }
    }

    IndexList m_fluent_offsets;
    IndexList m_derived_offsets;

    UnaryGroundActionList m_unary_actions;
    UnaryGroundAxiomList m_unary_axioms;

    PropositionList m_propositions;

    IndexList m_goal_propositions;

    std::deque<QueueEntry> m_queue;
};

class MaxHeuristic : public RelaxedPlanningGraph<MaxHeuristic>
{
public:
private:
    void initialize_annotations_impl();

    double extract_impl();

    bool on_expand_proposition_impl(const Proposition& proposition, DiscreteCost cost)
    {
        return m_proposition_annotations[proposition.get_index()].cost < cost;
    }

    void on_process_action_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAction& action);

    void on_process_axiom_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAxiom& axiom);

    friend class RelaxedPlanningGraph<MaxHeuristic>;

private:
    UnaryGroundActionAnnotationsList m_unary_action_states;
    UnaryGroundAxiomAnnotationsList m_unary_axiom_states;

    PropositionAnnotationsList<> m_proposition_annotations;
};

class AddHeuristic : public RelaxedPlanningGraph<AddHeuristic>
{
public:
private:
    void initialize_annotations_impl();

    double extract_impl();

    void on_process_action_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAction& action);

    void on_process_axiom_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAxiom& axiom);

    friend class RelaxedPlanningGraph<AddHeuristic>;

private:
    UnaryGroundActionAnnotationsList m_unary_action_states;
    UnaryGroundAxiomAnnotationsList m_unary_axiom_states;

    PropositionAnnotationsList<> m_proposition_annotations;
};

class FFHeuristic : public AddHeuristic
{
public:
private:
    void initialize_annotations_impl();

    double extract_impl();

    void on_process_action_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAction& action);

    void on_process_axiom_impl(const Proposition& proposition, DiscreteCost cost, const UnaryGroundAxiom& axiom);

    friend class RelaxedPlanningGraph<MaxHeuristic>;

private:
    UnaryGroundActionAnnotationsList m_unary_action_states;
    UnaryGroundAxiomAnnotationsList m_unary_axiom_states;

    PropositionAnnotationsList<bool> m_proposition_annotations;
};
}

#endif
