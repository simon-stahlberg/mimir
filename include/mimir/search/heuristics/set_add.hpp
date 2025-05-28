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

class SetAddHeuristicImpl : public rpg::RelaxedPlanningGraph<SetAddHeuristicImpl>
{
public:
    explicit SetAddHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation);

    static SetAddHeuristic create(const DeleteRelaxedProblemExplorator& delete_relaxation);

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
    /// Accumulate sets.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-action node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Action& action);

    /// @brief Update the "And"-axiom node.
    /// Choose maximal set among proposition and axiom.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-axiom node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Axiom& axiom);

    /// @brief Update the "Or"-proposition node.
    /// Choose maximal set among proposition and action achievers + action.
    /// @param action is the action.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Action& action, const rpg::Proposition& proposition);

    /// @brief Update the "Or"-proposition node.
    /// Choose maximal set among proposition and axiom achievers.
    /// @param axiom is the axiom.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Axiom& axiom, const rpg::Proposition& proposition);

    /// @brief Extract h_max heuristic estimate from the goal propositions.
    /// @return the h_max heuristic estimate.
    DiscreteCost extract_impl();

    friend class rpg::RelaxedPlanningGraph<SetAddHeuristicImpl>;

    rpg::AnnotationsList<IndexSet> m_setadd_action_annotations;
    rpg::AnnotationsList<IndexSet> m_setadd_axiom_annotations;

    rpg::AnnotationsList<IndexSet> m_setadd_proposition_annotations;

    IndexSet m_total_goal_annotations;

    static IndexSet& get_achievers(rpg::Annotations<IndexSet>& annotation) { return std::get<0>(annotation); }
    static const IndexSet& get_achievers(const rpg::Annotations<IndexSet>& annotation) { return std::get<0>(annotation); }
};

}

#endif
