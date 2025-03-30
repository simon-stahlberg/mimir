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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_INTERFACE_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir::search
{

/**
 * Dynamic interface class.
 */
class IAxiomEvaluator
{
public:
    virtual ~IAxiomEvaluator() = default;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(DenseState& dense_state) = 0;

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
