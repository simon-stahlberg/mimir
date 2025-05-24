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

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"

namespace mimir::search::rpg
{
class UnaryGroundAction
{
public:
    UnaryGroundAction(Index index,
                      const FlatIndexList& fluent_preconditions,
                      const FlatIndexList& derived_preconditions,
                      const FlatIndexList& cond_fluent_precondition,
                      const FlatIndexList& cond_derived_precondition,
                      Index fluent_effect);

    Index get_index() const;
    const FlatIndexList& get_fluent_precondition() const;
    const FlatIndexList& get_derived_precondition() const;
    const FlatIndexList& get_cond_fluent_precondition() const;
    const FlatIndexList& get_cond_derived_precondition() const;
    Index get_fluent_effect() const;

private:
    Index m_index;
    const FlatIndexList& m_fluent_precondition;
    const FlatIndexList& m_derived_precondition;
    const FlatIndexList& m_cond_fluent_precondition;
    const FlatIndexList& m_cond_derived_precondition;
    Index m_fluent_effect;
};
using UnaryGroundActionList = std::vector<UnaryGroundAction>;

template<typename... Ts>
struct UnaryGroundActionState
{
    size_t num_unsatisfied_preconditions;

    std::tuple<Ts...> m_data;
};

class UnaryGroundAxiom
{
public:
    UnaryGroundAxiom(Index index, const FlatIndexList& fluent_preconditions, const FlatIndexList& derived_preconditions, Index derived_effect);

    Index get_index() const;
    const FlatIndexList& get_fluent_precondition() const;
    const FlatIndexList& get_derived_precondition() const;
    Index get_fluent_effect() const;

private:
    Index m_index;
    const FlatIndexList& m_fluent_precondition;
    const FlatIndexList& m_derived_precondition;
    Index m_derived_effect;
};
using UnaryGroundAxiomList = std::vector<UnaryGroundAxiom>;

template<typename... Ts>
struct UnaryGroundAxiomState
{
    size_t num_unsatisfied_preconditions;

    std::tuple<Ts...> m_data;
};

class Proposition
{
public:
    Proposition(IndexList is_precondition_of_action, IndexList is_precondition_of_axiom, bool is_goal) :
        m_is_precondition_of_action(std::move(is_precondition_of_action)),
        m_is_precondition_of_axiom(std::move(is_precondition_of_axiom)),
        m_is_goal(is_goal)
    {
    }

    const IndexList& is_precondition_of_action() const;
    const IndexList& is_precondition_of_axiom() const;
    bool is_goal() const;

private:
    IndexList m_is_precondition_of_action;
    IndexList m_is_precondition_of_axiom;
    bool m_is_goal;
};

using PropositionList = std::vector<Proposition>;

template<typename... Annotations>
struct PropositionAnnotations
{
    ContinuousCost cost = std::numeric_limits<ContinuousCost>::infinity();
    std::tuple<Annotations...> m_annotations;
};

template<typename... Annotations>
using PropositionAnnotationsList = std::vector<PropositionAnnotations<Annotations...>>;

using FFPropositionAnnotations = PropositionAnnotations<bool>;
using FFPropositionAnnotationsList = std::vector<FFPropositionAnnotations>;

inline bool& is_marked(FFPropositionAnnotations& prop) { return std::get<0>(prop.m_annotations); }

template<typename Derived>
class RelaxedPlanningGraph : public IHeuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) : m_unary_actions(), m_unary_axioms()
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
            m_fluent_proposition_layer.emplace_back(
                std::move(is_fluent_precondition_of_action[atom->get_index()]),
                std::move(is_derived_precondition_of_axiom[atom->get_index]),
                delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::FluentTag>().get(atom->get_index()));
        }
        for (const auto& atom : delete_relaxation.create_ground_atoms<formalism::DerivedTag>())
        {
            m_derived_proposition_layer.emplace_back(
                std::move(is_fluent_precondition_of_action[atom->get_index()]),
                std::move(is_derived_precondition_of_axiom[atom->get_index]),
                delete_relaxation.get_problem()->get_positive_goal_atoms_bitset<formalism::DerivedTag>().get(atom->get_index()));
        }
    }

    double compute_heuristic(State state, bool is_goal_state) override
    {
        reset();
        brfs();
        return extract_impl();
    }

private:
    void reset() {}

    void brfs()
    {
        // TODO: dijkstra exploration
        auto num_unsat_goals = m_fluent_proposition_layer.size() + m_derived_goal_propositions.size();
        // TODO: apply axioms until none applicable
        // TODO: apply actions until none applicable
        while (!m_axiom_queue.empty() && !m_action_queue.empty()) {}
    }

private:
    UnaryGroundActionList m_unary_actions;
    UnaryGroundAxiomList m_unary_axioms;

protected:
    std::vector<Proposition> m_fluent_proposition_layer;
    std::vector<Proposition> m_derived_proposition_layer;

    IndexList m_fluent_goal_propositions;
    IndexList m_derived_goal_propositions;

    PriorityQueue<ContinuousCost, Index> m_action_queue;
    PriorityQueue<ContinuousCost, Index> m_axiom_queue;
};

class MaxHeuristic : public RelaxedPlanningGraph<MaxHeuristic>
{
public:
private:
};

class AddHeuristic : public RelaxedPlanningGraph<AddHeuristic>
{
public:
private:
};

class FFHeuristic : public AddHeuristic
{
public:
private:
};

}

#endif
