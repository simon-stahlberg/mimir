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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IApplicableActionGenerator
{
public:
    virtual ~IApplicableActionGenerator() = default;

    /// @brief Generate all applicable actions for a given state.
    virtual void generate_applicable_actions(State state, GroundActionList& out_applicable_actions) = 0;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(StateImpl& unextended_state) = 0;

    // Notify that a new f-layer was reached
    virtual void on_finish_search_layer() const = 0;

    /// @brief Notify that the search has finished
    virtual void on_end_search() const = 0;

    /// @brief Return the ground actions.
    virtual const GroundActionList& get_ground_actions() const = 0;

    /// @brief Return the ground action with the given index.
    virtual GroundAction get_ground_action(Index action_index) const = 0;

    /// @brief Return the ground axioms.
    virtual const GroundAxiomList& get_ground_axioms() const = 0;

    /// @brief Return the ground axiom with the given index.
    virtual GroundAxiom get_ground_axiom(Index action_index) const = 0;

    virtual size_t get_num_ground_actions() const = 0;
    virtual size_t get_num_ground_axioms() const = 0;

    /* Getters */
    virtual Problem get_problem() const = 0;
    virtual const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const = 0;
};

}

#endif
