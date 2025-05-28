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

#ifndef MIMIR_SEARCH_HEURISTICS_ADD_HPP_
#define MIMIR_SEARCH_HEURISTICS_ADD_HPP_

#include "mimir/search/heuristics/rpg_base.hpp"

namespace mimir::search
{

class AddHeuristicImpl : public rpg::RelaxedPlanningGraph<AddHeuristicImpl>
{
public:
    explicit AddHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation);

    static AddHeuristic create(const DeleteRelaxedProblemExplorator& delete_relaxation);

private:
    /// @brief Initialize "And"-structure node annotations.
    /// Sets the cost for each structure node to 0.
    void initialize_and_annotations_impl(const rpg::Action& action);
    void initialize_and_annotations_impl(const rpg::Axiom& axiom);

    /// @brief Initialize "Or"-proposition node annotations.
    /// Sets the cost for each proposition that is true in the state to 0, and otherwise to infinity.
    void initialize_or_annotations_impl(const rpg::Proposition& proposition);
    void initialize_or_annotations_and_queue_impl(const rpg::Proposition& proposition);

    /// @brief Update the "And"-action node.
    /// Accumulates costs.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-action node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Action& action);

    /// @brief Update the "And"-axiom node.
    /// Use maximal cost.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-axiom node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Axiom& axiom);

    /// @brief Update the "Or"-proposition node.
    /// Choose minimal cost among proposition cost and action cost + 1.
    /// @param action is the action.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Action& action, const rpg::Proposition& proposition);

    /// @brief Update the "Or"-proposition node.
    /// Choose minimal cost among proposition cost and axiom cost.
    /// @param axiom is the axiom.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Axiom& axiom, const rpg::Proposition& proposition);

    /// @brief Extract h_max heuristic estimate from the goal propositions.
    /// @return the h_max heuristic estimate.
    DiscreteCost extract_impl(State state);

    friend class rpg::RelaxedPlanningGraph<AddHeuristicImpl>;
};

}

#endif
