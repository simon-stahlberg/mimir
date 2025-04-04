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

#ifndef MIMIR_FORMALISM_GROUND_AXIOM_HPP_
#define MIMIR_FORMALISM_GROUND_AXIOM_HPP_

#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{
struct GroundEffectDerivedLiteral
{
    bool polarity = false;
    Index atom_index = Index(0);
};

class GroundAxiomImpl
{
private:
    Index m_index = Index(0);
    Index m_axiom_index = Index(0);
    FlatIndexList m_object_indices = FlatIndexList();
    GroundConjunctiveCondition m_conjunctive_condition = GroundConjunctiveCondition();
    GroundEffectDerivedLiteral m_literal = GroundEffectDerivedLiteral();

public:
    using FormalismEntity = void;

    Index& get_index();
    Index get_index() const;

    Index& get_axiom();
    Index get_axiom_index() const;

    FlatIndexList& get_object_indices();
    const FlatIndexList& get_object_indices() const;

    /* Conjunctive part */
    GroundConjunctiveCondition& get_conjunctive_condition();
    const GroundConjunctiveCondition& get_conjunctive_condition() const;

    /* Effect*/
    GroundEffectDerivedLiteral& get_derived_effect();
    const GroundEffectDerivedLiteral& get_derived_effect() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the lifted schema index and the binding because they imply the rest.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_axiom_index(), std::cref(get_object_indices())); }

    auto cista_members() noexcept { return std::tie(m_index, m_axiom_index, m_object_indices, m_conjunctive_condition, m_literal); }
};

/**
 * Mimir types
 */

using GroundAxiomImplSet = mimir::buffering::UnorderedSet<GroundAxiomImpl>;
}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundEffectDerivedLiteral, const formalism::ProblemImpl&>& data);

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundAxiom, const formalism::ProblemImpl&>& data);
}

#endif
