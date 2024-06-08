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

#include "mimir/formalism/formalism.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IAxiomEvaluator
{
public:
    virtual ~IAxiomEvaluator() = default;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) = 0;
};

/**
 * Type aliases
 */

using IAE = IAxiomEvaluator;

}

#endif
