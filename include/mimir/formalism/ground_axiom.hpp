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
class GroundAxiomImpl
{
private:
    Index m_index;
    Axiom m_axiom;
    ObjectList m_objects;
    GroundConjunctiveCondition m_conjunctive_condition;
    GroundLiteral<DerivedTag> m_literal;

    GroundAxiomImpl(Index index, Axiom axiom, ObjectList objects, GroundConjunctiveCondition conjunctive_condition, GroundLiteral<DerivedTag> literal);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundAxiomImpl(const GroundAxiomImpl& other) = delete;
    GroundAxiomImpl& operator=(const GroundAxiomImpl& other) = delete;
    GroundAxiomImpl(GroundAxiomImpl&& other) = default;
    GroundAxiomImpl& operator=(GroundAxiomImpl&& other) = default;

    Index get_index() const;
    Axiom get_axiom() const;
    const ObjectList& get_objects() const;
    GroundConjunctiveCondition get_conjunctive_condition() const;
    GroundLiteral<DerivedTag> get_literal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the lifted schema index and the binding because they imply the rest.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_axiom(), std::cref(get_objects())); }
};
}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundAxiom, const formalism::ProblemImpl&>& data);
}

#endif
