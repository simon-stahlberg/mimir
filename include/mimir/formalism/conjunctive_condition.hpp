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
    ParameterList m_parameters;
    LiteralLists<StaticTag, FluentTag, DerivedTag> m_literals;
    GroundLiteralLists<StaticTag, FluentTag, DerivedTag> m_nullary_ground_literals;
    NumericConstraintList m_numeric_constraints;
    GroundNumericConstraintList m_nullary_ground_numeric_constraints;

    // Below: add additional members if needed and initialize them in the constructor

    ConjunctiveConditionImpl(Index index,
                             ParameterList parameters,
                             LiteralLists<StaticTag, FluentTag, DerivedTag> literals,
                             GroundLiteralLists<StaticTag, FluentTag, DerivedTag> nullary_ground_literals,
                             NumericConstraintList numeric_constraints,
                             GroundNumericConstraintList nullary_ground_numeric_constraints);

    static auto identifying_args(const ParameterList& parameters,
                                 const LiteralLists<StaticTag, FluentTag, DerivedTag>& literals,
                                 const GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& nullary_ground_literals,
                                 const NumericConstraintList& numeric_constraints,
                                 const GroundNumericConstraintList& nullary_ground_numeric_constraints) noexcept
    {
        return std::tuple(std::cref(parameters),
                          std::cref(boost::hana::at_key(literals, boost::hana::type<StaticTag> {})),
                          std::cref(boost::hana::at_key(literals, boost::hana::type<FluentTag> {})),
                          std::cref(boost::hana::at_key(literals, boost::hana::type<DerivedTag> {})),
                          std::cref(numeric_constraints));
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    ConjunctiveConditionImpl(const ConjunctiveConditionImpl& other) = delete;
    ConjunctiveConditionImpl& operator=(const ConjunctiveConditionImpl& other) = delete;
    ConjunctiveConditionImpl(ConjunctiveConditionImpl&& other) = default;
    ConjunctiveConditionImpl& operator=(ConjunctiveConditionImpl&& other) = default;

    Index get_index() const;
    const ParameterList& get_parameters() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const LiteralList<P>& get_literals() const;
    const LiteralLists<StaticTag, FluentTag, DerivedTag>& get_hana_literals() const;
    template<IsStaticOrFluentOrDerivedTag P>
    const GroundLiteralList<P>& get_nullary_ground_literals() const;
    const GroundLiteralLists<StaticTag, FluentTag, DerivedTag>& get_hana_nullary_ground_literals() const;
    const NumericConstraintList& get_numeric_constraints() const;
    const GroundNumericConstraintList& get_nullary_ground_numeric_constraints() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept
    {
        return std::tuple(std::cref(get_parameters()),
                          std::cref(get_literals<StaticTag>()),
                          std::cref(get_literals<FluentTag>()),
                          std::cref(get_literals<DerivedTag>()),
                          std::cref(get_numeric_constraints()));
    }
};
}

#endif
