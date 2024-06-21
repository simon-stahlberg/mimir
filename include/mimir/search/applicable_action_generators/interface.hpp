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

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/state.hpp"

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
    virtual void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) = 0;

    // Notify that a new f-layer was reached
    virtual void on_finish_f_layer() const = 0;

    /// @brief Notify that the search has finished
    virtual void on_end_search() const = 0;

    /// @brief Return the action with the given id.
    [[nodiscard]] virtual GroundAction get_action(size_t action_id) const = 0;

    /* Getters */
    [[nodiscard]] virtual Problem get_problem() const = 0;
    [[nodiscard]] virtual const std::shared_ptr<PDDLFactories>& get_pddl_factories() const = 0;
};

/**
 * Type aliases
 */

using IAAG = IApplicableActionGenerator;

}

#endif
