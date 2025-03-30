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

#include "mimir/algorithms/generator.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/**
 * Dynamic interface class.
 */
class IApplicableActionGenerator
{
public:
    virtual ~IApplicableActionGenerator() = default;

    /// @brief Generate all applicable actions for a given state.
    virtual mimir::generator<formalism::GroundAction> create_applicable_action_generator(State state) = 0;

    /// @brief Generate all applicable actions for a bitset representation of a state.
    virtual mimir::generator<formalism::GroundAction> create_applicable_action_generator(const DenseState& dense_state) = 0;

    /// @brief Accumulate event handler statistics during search.
    virtual void on_finish_search_layer() = 0;
    virtual void on_end_search() = 0;

    /**
     * Getters
     */

    virtual const formalism::Problem& get_problem() const = 0;
};

}

#endif
