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
#include "mimir/search/heuristics/interface.hpp"

namespace mimir::search::rpg
{
class UnaryGroundAction
{
public:
    UnaryGroundAction(Index index,
                      const FlatIndexList* fluent_preconditions,
                      const FlatIndexList* derived_preconditions,
                      Index fluent_effect,
                      ContinuousCost cost);

    Index get_index() const;
    const FlatIndexList* get_fluent_precondition() const;
    const FlatIndexList* get_derived_precondition() const;
    Index get_fluent_effect() const;
    ContinuousCost get_cost() const;

private:
    Index m_index;
    const FlatIndexList* m_fluent_precondition;
    const FlatIndexList* m_derived_precondition;
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
    const FlatIndexList* get_fluent_precondition() const;
    const FlatIndexList* get_derived_precondition() const;
    Index get_fluent_effect() const;

private:
    Index m_index;
    const FlatIndexList* m_fluent_precondition;
    const FlatIndexList* m_derived_precondition;
    Index m_derived_effect;
};
using UnaryGroundAxiomList = std::vector<UnaryGroundAxiom>;

template<typename... Ts>
struct UnaryGroundAxiomState
{
    size_t num_unsatisfied_preconditions;

    std::tuple<Ts...> m_data;
};

template<formalism::IsFluentOrDerivedTag P, typename... Ts>
struct Proposition
{
    std::tuple<Ts...> m_data;

    const FlatIndexList* m_precondition_of_action;
    const FlatIndexList* m_precondition_of_axiom;
};
template<typename... Ts>
using PropositionLayer = std::vector<Proposition<Ts...>>;

/// @brief TODO: write generic traversal logic, forward calls to derived when specializations are necessary.
/// @tparam ...Ts
/// @tparam Derived
template<typename Derived, typename... Ts>
class RelaxedPlanningGraph : public Heuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation);

    double compute_heuristic(State state) override;

private:
    UnaryGroundActionList m_unary_actions;
    UnaryGroundAxiomList m_unary_axioms;

    PropositionLayer<formalism::FluentTag, Ts...> m_fluent_proposition_layer;
    PropositionLayer<formalism::DerivedTag, Ts...> m_derived_proposition_layer;
};
}

#endif
