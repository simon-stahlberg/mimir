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

namespace mimir::search::rpg
{

/**
 * UnaryGroundAction
 */

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
        m_num_preconditions(m_fluent_preconditions.size() + m_derived_preconditions.size() + m_cond_fluent_preconditions.size()
                            + m_cond_derived_preconditions.size()),
        m_fluent_effect(fluent_effect)
    {
    }

    Index get_index() const { return m_index; }
    const FlatIndexList& get_fluent_preconditions() const { return m_fluent_preconditions; }
    const FlatIndexList& get_derived_preconditions() const { return m_derived_preconditions; }
    const FlatIndexList& get_cond_fluent_preconditions() const { return m_cond_fluent_preconditions; }
    const FlatIndexList& get_cond_derived_preconditions() const { return m_cond_derived_preconditions; }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_fluent_effect() const { return m_fluent_effect; }

private:
    Index m_index;
    const FlatIndexList& m_fluent_preconditions;
    const FlatIndexList& m_derived_preconditions;
    const FlatIndexList& m_cond_fluent_preconditions;
    const FlatIndexList& m_cond_derived_preconditions;
    size_t m_num_preconditions;
    Index m_fluent_effect;
};
using UnaryGroundActionList = std::vector<UnaryGroundAction>;

/**
 * UnaryGroundAxiom
 */

class UnaryGroundAxiom
{
public:
    UnaryGroundAxiom(Index index, const FlatIndexList& fluent_preconditions, const FlatIndexList& derived_preconditions, Index derived_effect) :
        m_index(index),
        m_fluent_preconditions(fluent_preconditions),
        m_derived_preconditions(derived_preconditions),
        m_num_preconditions(m_fluent_preconditions.size() + m_derived_preconditions.size()),
        m_derived_effect(derived_effect)
    {
    }

    Index get_index() const { return m_index; }
    const FlatIndexList& get_fluent_precondition() const { return m_fluent_preconditions; }
    const FlatIndexList& get_derived_precondition() const { return m_derived_preconditions; }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_derived_effect() const { return m_derived_effect; }

private:
    Index m_index;
    const FlatIndexList& m_fluent_preconditions;
    const FlatIndexList& m_derived_preconditions;
    size_t m_num_preconditions;
    Index m_derived_effect;
};
using UnaryGroundAxiomList = std::vector<UnaryGroundAxiom>;

/**
 * Proposition
 */

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

/**
 * Annotations
 */

template<typename... T>
using Annotations = std::tuple<T...>;
template<typename... T>
using AnnotationsList = std::vector<Annotations<T...>>;

/**
 * RelaxedPlanningGraph
 */

template<typename Derived>
class RelaxedPlanningGraph : public IHeuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    friend Derived;

    static constexpr Index DUMMY_PROPOSITION_INDEX = 0;

public:
    ContinuousCost compute_heuristic(State state, bool is_goal_state) override
    {
        initialize_annotations_and_queue(state);              ///< base annotations
        self().initialize_annotations_and_queue_impl(state);  ///< derived annotations
        dijksta();
        const auto cost = self().extract_impl();
        // std::cout << "Cost: " << cost << std::endl;
        return (cost == MAX_DISCRETE_COST) ? INFINITY_CONTINUOUS_COST : cost;
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
        auto trivial_unary_actions = IndexList {};

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
                    for (const auto& pre_atom_index : cond_effect->get_conjunctive_condition()->get_positive_precondition<formalism::FluentTag>())
                    {
                        is_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : cond_effect->get_conjunctive_condition()->get_positive_precondition<formalism::DerivedTag>())
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

                    if (m_unary_actions.back().get_num_preconditions() == 0)
                    {
                        trivial_unary_actions.push_back(unary_action_index);
                    }
                }
            }
        }

        auto is_fluent_precondition_of_axiom = IndexMap<IndexList> {};
        auto is_derived_precondition_of_axiom = IndexMap<IndexList> {};
        auto trivial_unary_axioms = IndexList {};

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
                                                      axiom->get_literal()->get_atom()->get_index()));

            if (m_unary_axioms.back().get_num_preconditions() == 0)
            {
                trivial_unary_axioms.push_back(unary_axiom_index);
            }
        }

        // std::cout << "Num trivial actions: " << trivial_unary_actions.size() << std::endl;
        // std::cout << "Num trivial axioms: " << trivial_unary_axioms.size() << std::endl;

        // We create a dummy proposition with index 0 that contains all trivial unary actions and axioms without preconditions.
        m_propositions.emplace_back(m_propositions.size(), std::move(trivial_unary_actions), std::move(trivial_unary_axioms), false);

        for (const auto& atom : delete_relaxation.create_ground_atoms<formalism::FluentTag>())
        {
            const auto proposition_index = m_propositions.size();

            // std::cout << proposition_index << " " << *atom << std::endl;

            if (atom->get_index() >= m_fluent_offsets.size())
            {
                m_fluent_offsets.resize(atom->get_index() + 1, MAX_INDEX);
            }
            m_fluent_offsets[atom->get_index()] = proposition_index;

            const auto is_goal = delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::FluentTag>().get(atom->get_index());

            m_propositions.emplace_back(proposition_index,
                                        std::move(is_fluent_precondition_of_action[atom->get_index()]),
                                        std::move(is_derived_precondition_of_axiom[atom->get_index()]),
                                        is_goal);

            if (is_goal)
                m_goal_propositions.push_back(proposition_index);
        }
        for (const auto& atom : delete_relaxation.create_ground_atoms<formalism::DerivedTag>())
        {
            const auto proposition_index = m_propositions.size();

            // std::cout << proposition_index << " " << *atom << std::endl;

            if (atom->get_index() >= m_derived_offsets.size())
            {
                m_derived_offsets.resize(atom->get_index() + 1, MAX_INDEX);
            }
            m_derived_offsets[atom->get_index()] = proposition_index;

            const auto is_goal = delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::DerivedTag>().get(atom->get_index());

            m_propositions.emplace_back(proposition_index,
                                        std::move(is_fluent_precondition_of_action[atom->get_index()]),
                                        std::move(is_derived_precondition_of_axiom[atom->get_index()]),
                                        delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::DerivedTag>().get(atom->get_index()));

            if (is_goal)
                m_goal_propositions.push_back(proposition_index);
        }
    }

    void initialize_annotations_and_queue(State state)
    {
        m_action_annotations.resize(this->m_unary_actions.size());
        for (size_t i = 0; i < this->m_unary_actions.size(); ++i)
        {
            auto& annotation = m_action_annotations[i];
            const auto& action = this->m_unary_actions[i];
            get_cost(annotation) = 0;
            get_num_unsatisfied_preconditions(annotation) = action.get_num_preconditions();
        }
        m_axiom_annotations.resize(this->m_unary_axioms.size());
        for (size_t i = 0; i < this->m_unary_axioms.size(); ++i)
        {
            auto& annotation = m_axiom_annotations[i];
            const auto& axiom = this->m_unary_axioms[i];
            get_cost(annotation) = 0;
            get_num_unsatisfied_preconditions(annotation) = axiom.get_num_preconditions();
        }
        m_proposition_annotations.resize(this->m_propositions.size());
        for (size_t i = 0; i < this->m_propositions.size(); ++i)
        {
            auto& annotation = m_proposition_annotations[i];
            get_cost(annotation) = MAX_DISCRETE_COST;
        }

        m_queue.clear();

        for (const auto atom_index : state->get_atoms<formalism::FluentTag>())
        {
            const auto proposition_index = this->m_fluent_offsets[atom_index];
            auto& annotation = m_proposition_annotations[proposition_index];
            get_cost(annotation) = 0;
            m_queue.insert(0, QueueEntry { proposition_index, 0 });
        }
        for (const auto atom_index : state->get_atoms<formalism::DerivedTag>())
        {
            const auto proposition_index = this->m_derived_offsets[atom_index];
            auto& annotation = m_proposition_annotations[proposition_index];
            get_cost(annotation) = 0;
            m_queue.insert(0, QueueEntry { proposition_index, 0 });
        }
        // Trivial dummy proposition to trigger actions and axioms without preconditions
        auto& annotation = m_proposition_annotations[0];
        get_cost(annotation) = 0;
        m_queue.insert(0, QueueEntry { 0, 0 });
    }

    void on_process_action(const Proposition& proposition, const UnaryGroundAction& action)
    {
        auto& action_annotation = m_action_annotations[action.get_index()];

        self().update_and_annotation_impl(proposition, action);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(action_annotation);
        }
        if (get_num_unsatisfied_preconditions(action_annotation) == 0)
        {
            const auto effect_proposition_index = this->m_fluent_offsets[action.get_fluent_effect()];
            const auto& effect_proposition = this->m_propositions[effect_proposition_index];

            self().update_or_annotation_impl(action, effect_proposition);
        }
    }

    void on_process_axiom(const Proposition& proposition, const UnaryGroundAxiom& axiom)
    {
        auto& axiom_annotation = m_axiom_annotations[axiom.get_index()];

        self().update_and_annotation_impl(proposition, axiom);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(axiom_annotation);
        }
        if (get_num_unsatisfied_preconditions(axiom_annotation) == 0)
        {
            const auto effect_proposition_index = this->m_derived_offsets[axiom.get_derived_effect()];
            const auto& effect_proposition = this->m_propositions[effect_proposition_index];

            self().update_or_annotation_impl(axiom, effect_proposition);
        }
    }

    void dijksta()
    {
        auto num_unsat_goals = m_goal_propositions.size();

        while (!m_queue.empty())
        {
            const auto entry = m_queue.top();
            m_queue.pop();

            const auto& proposition = m_propositions[entry.proposition_index];
            const auto& annotation = m_proposition_annotations[entry.proposition_index];

            // std::cout << "Queue pop: " << entry.proposition_index << " " << entry.cost << std::endl;

            if (get_cost(annotation) < entry.cost)
            {
                continue;
            }

            if (proposition.is_goal() && --num_unsat_goals == 0)
            {
                return;
            }

            for (const auto action_index : proposition.is_precondition_of_actions())
            {
                const auto& action = m_unary_actions[action_index];

                self().on_process_action(proposition, action);
            }
            for (const auto axiom_index : proposition.is_precondition_of_axioms())
            {
                const auto& axiom = m_unary_axioms[axiom_index];

                self().on_process_axiom(proposition, axiom);
            }
        }

        // std::cout << "Num unsat goals: " << num_unsat_goals << std::endl;
    }

    IndexList m_fluent_offsets;
    IndexList m_derived_offsets;

    UnaryGroundActionList m_unary_actions;
    UnaryGroundAxiomList m_unary_axioms;

    AnnotationsList<DiscreteCost, size_t> m_action_annotations;
    AnnotationsList<DiscreteCost, size_t> m_axiom_annotations;

    static DiscreteCost& get_cost(Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static size_t& get_num_unsatisfied_preconditions(Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }
    static size_t get_num_unsatisfied_preconditions(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }

    PropositionList m_propositions;

    AnnotationsList<DiscreteCost> m_proposition_annotations;

    static DiscreteCost& get_cost(Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }

    IndexList m_goal_propositions;

    struct QueueEntry
    {
        Index proposition_index;
        DiscreteCost cost;
    };

    PriorityQueue<DiscreteCost, QueueEntry> m_queue;
};

/**
 * HMax
 */

class MaxHeuristicImpl : public RelaxedPlanningGraph<MaxHeuristicImpl>
{
public:
    explicit MaxHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : RelaxedPlanningGraph<MaxHeuristicImpl>(delete_relaxation) {}

    static MaxHeuristic create(const DeleteRelaxedProblemExplorator& delete_relaxation) { return std::make_shared<MaxHeuristicImpl>(delete_relaxation); }

private:
    /// @brief Additional initialization logic. Not needed for h_max.
    /// @param state is the state
    void initialize_annotations_and_queue_impl(State state)
    {
        // Nothing to be done in h_max.
    }

    /// @brief Extract h_max heuristic estimate from the goal propositions.
    /// @return the h_max heuristic estimate.
    DiscreteCost extract_impl()
    {
        auto total_cost = DiscreteCost(0);
        for (const auto proposition_index : this->m_goal_propositions)
        {
            const auto& annotation = this->m_proposition_annotations[proposition_index];
            if (get_cost(annotation) == MAX_DISCRETE_COST)
            {
                return MAX_DISCRETE_COST;
            }
            total_cost = std::max(total_cost, get_cost(annotation));
        }
        return total_cost;
    }

    /// @brief Update the "And"-action node with maximal cost.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-action node.
    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action)
    {
        auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
        auto& action_annotation = this->m_action_annotations[action.get_index()];

        get_cost(action_annotation) = std::max(get_cost(action_annotation), get_cost(proposition_annotation) + 1);  // actions assume unit cost 1.
    }

    /// @brief Update the "And"-axiom node with maximal cost.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-axiom node.
    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom)
    {
        auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];
        auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];

        get_cost(axiom_annotation) = std::max(get_cost(axiom_annotation), get_cost(proposition_annotation));
    }

    /// @brief Update the "Or"-proposition node with minimal cost and enqueue the proposition if cost is updated.
    /// @param action is the action.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition)
    {
        const auto& action_annotation = this->m_action_annotations[action.get_index()];
        auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

        if (get_cost(action_annotation) < get_cost(proposition_annotation))
        {
            get_cost(proposition_annotation) = get_cost(action_annotation);
            this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
        }
    }

    /// @brief Update the "Or"-proposition node with minimal cost and enqueue the proposition if cost is updated.
    /// @param axiom is the axiom.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition)
    {
        const auto& axiom_annotation = this->m_axiom_annotations[axiom.get_index()];
        auto& proposition_annotation = this->m_proposition_annotations[proposition.get_index()];

        if (get_cost(axiom_annotation) < get_cost(proposition_annotation))
        {
            get_cost(proposition_annotation) = get_cost(axiom_annotation);
            this->m_queue.insert(get_cost(proposition_annotation), QueueEntry { proposition.get_index(), get_cost(proposition_annotation) });
        }
    }

    friend class RelaxedPlanningGraph<MaxHeuristicImpl>;
};

/**
 * HAdd
 */

class AddHeuristicImpl : public RelaxedPlanningGraph<AddHeuristicImpl>
{
public:
private:
    void initialize_annotations_and_queue_impl(State state) {}

    DiscreteCost extract_impl();

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action) {}

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom) {}

    void update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition) {}

    void update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition) {}

    friend class RelaxedPlanningGraph<AddHeuristicImpl>;
};

/**
 * HSetAdd
 */

class SetAddHeuristicImpl : public RelaxedPlanningGraph<SetAddHeuristicImpl>
{
public:
private:
    void initialize_annotations_and_queue_impl(State state) {}

    DiscreteCost extract_impl();

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action) {}

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom) {}

    void update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition) {}

    void update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition) {}

    friend class RelaxedPlanningGraph<SetAddHeuristicImpl>;

private:
    AnnotationsList<IndexSet> m_ff_action_annotations;
    AnnotationsList<IndexSet> m_ff_axiom_annotations;

    AnnotationsList<IndexSet> m_sa_proposition_annotations;

    static IndexSet& get_achiever_set(Annotations<IndexSet>& annotation) { return std::get<0>(annotation); }
    static const IndexSet& get_achiever_set(const Annotations<IndexSet>& annotation) { return std::get<0>(annotation); }
};

/**
 * HFF
 */

class FFHeuristicImpl : public RelaxedPlanningGraph<FFHeuristicImpl>
{
public:
private:
    void initialize_annotations_and_queue_impl(State state) {}

    DiscreteCost extract_impl();

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAction& action) {}

    void update_and_annotation_impl(const Proposition& proposition, const UnaryGroundAxiom& axiom) {}

    void update_or_annotation_impl(const UnaryGroundAction& action, const Proposition& proposition) {}

    void update_or_annotation_impl(const UnaryGroundAxiom& axiom, const Proposition& proposition) {}

    friend class RelaxedPlanningGraph<FFHeuristicImpl>;

private:
    AnnotationsList<Index> m_ff_action_annotations;
    AnnotationsList<Index> m_ff_axiom_annotations;

    static Index& get_achiever(Annotations<Index>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const Annotations<Index>& annotation) { return std::get<0>(annotation); }

    AnnotationsList<Index, bool> m_ff_proposition_annotations;

    static Index& get_achiever(Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static bool& is_marked(Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
    static bool is_marked(const Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
};
}

#endif
