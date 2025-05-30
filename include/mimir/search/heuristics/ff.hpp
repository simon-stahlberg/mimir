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

#ifndef MIMIR_SEARCH_HEURISTICS_FF_HPP_
#define MIMIR_SEARCH_HEURISTICS_FF_HPP_

#include "mimir/search/heuristics/rpg_base.hpp"

namespace mimir::search
{

using FFStructuresAnnotations = HanaContainer<rpg::AnnotationsList<Index>, rpg::Action, rpg::Axiom>;

template<rpg::IsStructure S>
inline auto& get(FFStructuresAnnotations& annotations)
{
    return boost::hana::at_key(annotations, boost::hana::type<S> {});
}

class FFHeuristicImpl : public rpg::RelaxedPlanningGraph<FFHeuristicImpl>
{
public:
    explicit FFHeuristicImpl(const DeleteRelaxedProblemExplorator& delete_relaxation);

    static FFHeuristic create(const DeleteRelaxedProblemExplorator& delete_relaxation);

private:
    /**
     * The initialize and update step closely follows the `AddHeuristic`.
     */

    /// @brief Initialize "And"-structure node annotations.
    /// Sets the cost for each structure node to 0.
    void initialize_and_annotations_impl(const rpg::Action& action);
    void initialize_and_annotations_impl(const rpg::Axiom& axiom);

    /// @brief Initialize "Or"-proposition node annotations.
    /// Sets the cost for each prosition that is true in the state to 0, and otherwise to infinity.
    void initialize_or_annotations_impl(const rpg::Proposition& proposition);
    void initialize_or_annotations_and_queue_impl(const rpg::Proposition& proposition);

    /// @brief Update the "And"-action node.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-action node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Action& action);

    /// @brief Update the "And"-axiom node.
    /// @param proposition is the proposition.
    /// @param axiom is the "And"-axiom node.
    void update_and_annotation_impl(const rpg::Proposition& proposition, const rpg::Axiom& axiom);

    /// @brief Update the "Or"-proposition node.
    /// @param action is the action.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Action& action, const rpg::Proposition& proposition);

    /// @brief Update the "Or"-proposition node.
    /// @param axiom is the axiom.
    /// @param proposition is the "Or"-proposition node.
    void update_or_annotation_impl(const rpg::Axiom& axiom, const rpg::Proposition& proposition);

    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    void extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Action& action);
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    void extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Axiom& axiom);
    void extract_relaxed_plan_and_preferred_operators_recursively(State state, const rpg::Proposition& proposition);

    /// @brief Extract h_max heuristic estimate from the goal propositions.
    /// @return the h_max heuristic estimate.
    DiscreteCost extract_impl(State state);

    friend class rpg::RelaxedPlanningGraph<FFHeuristicImpl>;

private:
    FFStructuresAnnotations m_ff_structure_annotations;

    auto& get_ff_structures_annotations() { return m_ff_structure_annotations; }

    static Index& get_achiever(rpg::Annotations<Index>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const rpg::Annotations<Index>& annotation) { return std::get<0>(annotation); }

    rpg::AnnotationsList<Index, bool> m_ff_proposition_annotations;

    auto& get_ff_proposition_annotations() { return m_ff_proposition_annotations; }

    formalism::GroundActionSet m_relaxed_plan;

    auto& get_relaxed_plan() { return m_relaxed_plan; }

    static Index& get_achiever(rpg::Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static Index get_achiever(const rpg::Annotations<Index, bool>& annotation) { return std::get<0>(annotation); }
    static bool& is_marked(rpg::Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
    static bool is_marked(const rpg::Annotations<Index, bool>& annotation) { return std::get<1>(annotation); }
};

}

#endif
