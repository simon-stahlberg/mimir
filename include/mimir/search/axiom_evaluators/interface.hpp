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
#include "mimir/search/actions.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IDynamicAE
{
public:
    virtual ~IDynamicAE() = default;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) = 0;
};

/**
 * Static interface class.
 */
template<typename Derived_>
class IStaticAE : public IDynamicAE
{
private:
    IStaticAE() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    void generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms, FlatBitsetBuilder<Derived>& ref_derived_state_atoms) override
    {  //
        self().generate_and_apply_axioms_impl(fluent_state_atoms, ref_derived_state_atoms);
    }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsStateTag S>
class AE : public IStaticAE<AE<S>>
{
};

}

#endif
