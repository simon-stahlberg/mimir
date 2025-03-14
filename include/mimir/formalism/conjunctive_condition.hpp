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

#ifndef MIMIR_FORMALISM_CONJUNCTIVE_CONDITION_HPP_
#define MIMIR_FORMALISM_CONJUNCTIVE_CONDITION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{
class ConjunctiveConditionImpl
{
private:
    Index m_index;
    VariableList m_parameters;
    LiteralLists<Static, Fluent, Derived> m_literals;
    GroundLiteralLists<Static, Fluent, Derived> m_nullary_ground_literals;
    NumericConstraintList m_numeric_constraints;

    // Below: add additional members if needed and initialize them in the constructor

    ConjunctiveConditionImpl(Index index,
                             VariableList parameters,
                             LiteralLists<Static, Fluent, Derived> literals,
                             GroundLiteralLists<Static, Fluent, Derived> nullary_ground_literals,
                             NumericConstraintList numeric_constraints);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using PDDLEntity = void;

    // moveable but not copyable
    ConjunctiveConditionImpl(const ConjunctiveConditionImpl& other) = delete;
    ConjunctiveConditionImpl& operator=(const ConjunctiveConditionImpl& other) = delete;
    ConjunctiveConditionImpl(ConjunctiveConditionImpl&& other) = default;
    ConjunctiveConditionImpl& operator=(ConjunctiveConditionImpl&& other) = default;

    Index get_index() const;
    const VariableList& get_parameters() const;
    template<StaticOrFluentOrDerived P>
    const LiteralList<P>& get_literals() const;
    const LiteralLists<Static, Fluent, Derived>& get_hana_literals() const;
    template<StaticOrFluentOrDerived P>
    const GroundLiteralList<P>& get_nullary_ground_literals() const;
    const GroundLiteralLists<Static, Fluent, Derived>& get_hana_nullary_ground_literals() const;
    const NumericConstraintList& get_numeric_constraints() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::tuple(std::cref(get_parameters()),
                          std::cref(get_literals<Static>()),
                          std::cref(get_literals<Fluent>()),
                          std::cref(get_literals<Derived>()),
                          std::cref(get_numeric_constraints()));
    }
};

extern std::ostream& operator<<(std::ostream& out, const ConjunctiveConditionImpl& element);

extern std::ostream& operator<<(std::ostream& out, ConjunctiveCondition element);

}

#endif
