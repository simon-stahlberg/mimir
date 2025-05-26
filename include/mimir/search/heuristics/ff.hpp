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

#ifndef MIMIR_SEARCH_HEURISTICS_SET_ADD_HPP_
#define MIMIR_SEARCH_HEURISTICS_SET_ADD_HPP_

#include "mimir/search/heuristics/rpg_base.hpp"

namespace mimir::search
{

class FFHeuristicImpl : public rpg::RelaxedPlanningGraph<FFHeuristicImpl>
{
public:
    explicit FFHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation) : rpg::RelaxedPlanningGraph<FFHeuristicImpl>(delete_relaxation) {}

    static FFHeuristic create(const DeleteRelaxedProblemExplorator& delete_relaxation) { return std::make_shared<FFHeuristicImpl>(delete_relaxation); }

private:
    /// @brief Initialize "And"-structure node annotations.
    /// Sets the cost for each structure node to 0.
    void initialize_and_annotations_impl();

    /// @brief Initialize "Or"-proposition node annotations.
    /// Sets the cost for each prosition that is true in the state to 0, and otherwise to infinity.
    /// @param state is the state.
    void initialize_or_annotations_impl(State state);

    /// @brief Update the "And"-action node with maximal cost.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-action node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::UnaryGroundAction& action);

    /// @brief Update the "And"-axiom node with maximal cost.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-axiom node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::UnaryGroundAxiom& axiom);

    /// @brief Update the "Or"-proposition node with minimal cost and enqueue the proposition if cost is updated.
    /// @param action is the action.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::UnaryGroundAction& action, const rpg::Proposition& proposition);

    /// @brief Update the "Or"-proposition node with minimal cost and enqueue the proposition if cost is updated.
    /// @param axiom is the axiom.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::UnaryGroundAxiom& axiom, const rpg::Proposition& proposition);

    /// @brief Extract h_max heuristic estimate from the goal propositions.
    /// @return the h_max heuristic estimate.
    DiscreteCost extract_impl();

    friend class rpg::RelaxedPlanningGraph<FFHeuristicImpl>;

private:
    rpg::AnnotationsList<Index> m_ff_action_annotations;
    rpg::AnnotationsList<Index> m_ff_axiom_annotations;

    static Index& get_achiever(rpg::Annotations<Index>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const rpg::Annotations<Index>& annotation) { return std::get<0>(annotation); }

    rpg::AnnotationsList<Index, bool> m_ff_proposition_annotations;

    static Index& get_achiever(rpg::Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const rpg::Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static bool& is_marked(rpg::Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
    static bool is_marked(const rpg::Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
};

}

#endif
