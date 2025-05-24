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
                      const std::optional<std::reference_wrapper<const FlatIndexList>>& cond_fluent_precondition,
                      const std::optional<std::reference_wrapper<const FlatIndexList>>& cond_derived_precondition,
                      Index fluent_effect,
                      ContinuousCost cost);

    Index get_index() const;
    const FlatIndexList& get_fluent_precondition() const;
    const FlatIndexList& get_derived_precondition() const;
    Index get_fluent_effect() const;
    ContinuousCost get_cost() const;

private:
    Index m_index;
    const FlatIndexList& m_fluent_precondition;
    const FlatIndexList& m_derived_precondition;
    std::optional<std::reference_wrapper<const FlatIndexList>> m_cond_fluent_precondition;
    std::optional<std::reference_wrapper<const FlatIndexList>> m_cond_derived_precondition;
    Index m_fluent_effect;
    ContinuousCost m_cost;
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
    UnaryGroundAxiom(Index index, const FlatIndexList* fluent_preconditions, const FlatIndexList* derived_preconditions, Index derived_effect);

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

template<typename T>
concept IsProposition = requires(T a) {
    { a.cost } -> std::same_as<ContinuousCost&>;
    { a.is_goal } -> std::same_as<bool&>;
    { a.precondition_of_action } -> std::same_as<const FlatIndexList*&>;
    { a.precondition_of_axiom } -> std::same_as<const FlatIndexList*&>;
};

template<typename... Annotations>
struct BaseProposition
{
    ContinuousCost cost = std::numeric_limits<ContinuousCost>::infinity();
    bool is_goal = false;
    const FlatIndexList* precondition_of_action = nullptr;
    const FlatIndexList* precondition_of_axiom = nullptr;

    std::tuple<Annotations...> m_annotations;

    BaseProposition() = default;
};

static_assert(IsProposition<BaseProposition<>>);

using FFProposition = BaseProposition<bool>;

inline bool& is_marked(FFProposition& prop) { return std::get<0>(prop.m_annotations); }

template<typename Derived, IsProposition Proposition>
class RelaxedPlanningGraph : public IHeuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) : m_unary_actions(), m_unary_axioms()
    {
        for (const auto& action : delete_relaxation.create_ground_actions())
        {
            for (const auto& atom_index : action->get_conjunctive_effect()->get_positive_effects())
            {
                m_unary_actions.push_back(UnaryGroundAction(m_unary_actions.size(),
                                                            &action->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::FluentTag>(),
                                                            &action->get_conjunctive_condition()->get_compressed_positive_precondition<formalism::DerivedTag>(),
                                                            atom_index));
            }
            for (const auto& cond_effect : action->get_conditional_effects())
            {
                for (const auto& atom_index : cond_effect->get_conjunctive_effect()->get_positive_effects()) {}
            }
        }
    }

    double compute_heuristic(State state, bool is_goal_state) override
    {
        reset();
        dijkstra();
        return extract_impl();
    }

private:
    void reset() {}

    void dijkstra()
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

template<IsProposition Proposition>
class MaxHeuristicTemplate : public RelaxedPlanningGraph<MaxHeuristicTemplate<Proposition>, Proposition>
{
public:
private:
};

using MaxHeuristic = MaxHeuristicTemplate<BaseProposition<>>;

template<IsProposition Proposition>
class AddHeuristicTemplate : public RelaxedPlanningGraph<AddHeuristicTemplate<Proposition>, Proposition>
{
public:
private:
};

using AddHeuristic = AddHeuristicTemplate<BaseProposition<>>;

class FFHeuristic : public AddHeuristicTemplate<FFProposition>
{
public:
private:
};

}

#endif
