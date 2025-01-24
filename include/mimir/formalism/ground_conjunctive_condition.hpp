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

#ifndef MIMIR_FORMALISM_GROUND_CONJUNCTIVE_CONDITION_HPP_
#define MIMIR_FORMALISM_GROUND_CONJUNCTIVE_CONDITION_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/dense_state.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir
{

class GroundConjunctiveCondition
{
private:
    FlatIndexList m_positive_static_atoms = FlatIndexList();
    FlatIndexList m_negative_static_atoms = FlatIndexList();
    FlatIndexList m_positive_fluent_atoms = FlatIndexList();
    FlatIndexList m_negative_fluent_atoms = FlatIndexList();
    FlatIndexList m_positive_derived_atoms = FlatIndexList();
    FlatIndexList m_negative_derived_atoms = FlatIndexList();
    FlatExternalPtrList<const GroundNumericConstraintImpl> m_numeric_constraints = FlatExternalPtrList<const GroundNumericConstraintImpl>();

public:
    template<PredicateTag P>
    FlatIndexList& get_positive_precondition();
    template<PredicateTag P>
    const FlatIndexList& get_positive_precondition() const;

    template<PredicateTag P>
    FlatIndexList& get_negative_precondition();
    template<PredicateTag P>
    const FlatIndexList& get_negative_precondition() const;

    FlatExternalPtrList<const GroundNumericConstraintImpl>& get_numeric_constraints();
    const FlatExternalPtrList<const GroundNumericConstraintImpl>& get_numeric_constraints() const;

    template<PredicateTag P>
    bool is_applicable(const FlatBitset& atoms) const;

    bool is_applicable(const FlatDoubleList& fluent_numeric_variables) const;

    bool is_dynamically_applicable(const DenseState& dense_state) const;

    bool is_statically_applicable(const FlatBitset& static_positive_atoms) const;

    bool is_applicable(Problem problem, const DenseState& dense_state) const;

    auto cista_members() noexcept
    {
        return std::tie(m_positive_static_atoms,
                        m_negative_static_atoms,
                        m_positive_fluent_atoms,
                        m_negative_fluent_atoms,
                        m_positive_derived_atoms,
                        m_negative_derived_atoms,
                        m_numeric_constraints);
    }
};

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConjunctiveCondition, const PDDLRepositories&>& data);

}

#endif
