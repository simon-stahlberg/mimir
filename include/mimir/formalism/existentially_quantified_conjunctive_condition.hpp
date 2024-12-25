/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_EXISTENTIALLY_QUANTIFIED_CONJUNCTIVE_CONDITION_HPP_
#define MIMIR_FORMALISM_EXISTENTIALLY_QUANTIFIED_CONJUNCTIVE_CONDITION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ExistentiallyQuantifiedConjunctiveConditionImpl
{
private:
    Index m_index;
    VariableList m_parameters;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    GroundLiteralList<Static> m_nullary_static_conditions;
    GroundLiteralList<Fluent> m_nullary_fluent_conditions;
    GroundLiteralList<Derived> m_nullary_derived_conditions;

    // Below: add additional members if needed and initialize them in the constructor

    ExistentiallyQuantifiedConjunctiveConditionImpl(Index index,
                                                    VariableList parameters,
                                                    LiteralList<Static> static_conditions,
                                                    LiteralList<Fluent> fluent_conditions,
                                                    LiteralList<Derived> derived_conditions,
                                                    GroundLiteralList<Static> nullary_static_conditions,
                                                    GroundLiteralList<Fluent> nullary_fluent_conditions,
                                                    GroundLiteralList<Derived> nullary_derived_conditions);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ExistentiallyQuantifiedConjunctiveConditionImpl(const ExistentiallyQuantifiedConjunctiveConditionImpl& other) = delete;
    ExistentiallyQuantifiedConjunctiveConditionImpl& operator=(const ExistentiallyQuantifiedConjunctiveConditionImpl& other) = delete;
    ExistentiallyQuantifiedConjunctiveConditionImpl(ExistentiallyQuantifiedConjunctiveConditionImpl&& other) = default;
    ExistentiallyQuantifiedConjunctiveConditionImpl& operator=(ExistentiallyQuantifiedConjunctiveConditionImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    template<PredicateTag P>
    const LiteralList<P>& get_literals() const;
    template<PredicateTag P>
    const GroundLiteralList<P>& get_nullary_ground_literals() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_parameters),
                                     std::as_const(m_static_conditions),
                                     std::as_const(m_fluent_conditions),
                                     std::as_const(m_derived_conditions));
    }
};

extern std::ostream& operator<<(std::ostream& out, const ExistentiallyQuantifiedConjunctiveConditionImpl& element);

extern std::ostream& operator<<(std::ostream& out, ExistentiallyQuantifiedConjunctiveCondition element);

}

#endif
