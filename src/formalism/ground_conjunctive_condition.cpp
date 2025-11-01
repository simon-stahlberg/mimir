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

#include "mimir/formalism/ground_conjunctive_condition.hpp"

#include "mimir/common/declarations.hpp"
#include "mimir/common/formatter.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/problem.hpp"

#include <ostream>
#include <tuple>

namespace mimir::formalism
{

/* GroundConjunctiveCondition */

GroundConjunctiveConditionImpl::GroundConjunctiveConditionImpl(
    Index index,
    HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> preconditions,
    GroundNumericConstraintList numeric_constraints) :
    m_index(index),
    m_preconditions(preconditions),
    m_numeric_constraints(std::move(numeric_constraints))
{
    assert((get_compressed_precondition<PositiveTag, StaticTag>()->is_compressed()));
    assert((get_compressed_precondition<PositiveTag, FluentTag>()->is_compressed()));
    assert((get_compressed_precondition<PositiveTag, DerivedTag>()->is_compressed()));
    assert((get_compressed_precondition<NegativeTag, StaticTag>()->is_compressed()));
    assert((get_compressed_precondition<NegativeTag, FluentTag>()->is_compressed()));
    assert((get_compressed_precondition<NegativeTag, DerivedTag>()->is_compressed()));

    assert((std::is_sorted(get_compressed_precondition<PositiveTag, StaticTag>()->compressed_begin(),
                           get_compressed_precondition<PositiveTag, StaticTag>()->compressed_end())));
    assert((std::is_sorted(get_compressed_precondition<PositiveTag, FluentTag>()->compressed_begin(),
                           get_compressed_precondition<PositiveTag, FluentTag>()->compressed_end())));
    assert((std::is_sorted(get_compressed_precondition<PositiveTag, DerivedTag>()->compressed_begin(),
                           get_compressed_precondition<PositiveTag, DerivedTag>()->compressed_end())));
    assert((std::is_sorted(get_compressed_precondition<NegativeTag, StaticTag>()->compressed_begin(),
                           get_compressed_precondition<NegativeTag, StaticTag>()->compressed_end())));
    assert((std::is_sorted(get_compressed_precondition<NegativeTag, FluentTag>()->compressed_begin(),
                           get_compressed_precondition<NegativeTag, FluentTag>()->compressed_end())));
    assert((std::is_sorted(get_compressed_precondition<NegativeTag, DerivedTag>()->compressed_begin(),
                           get_compressed_precondition<NegativeTag, DerivedTag>()->compressed_end())));
}

Index GroundConjunctiveConditionImpl::get_index() const { return m_index; }

template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition() const
{
    return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
}

template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<PositiveTag, StaticTag>() const;
template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<PositiveTag, FluentTag>() const;
template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<PositiveTag, DerivedTag>() const;
template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<NegativeTag, StaticTag>() const;
template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<NegativeTag, FluentTag>() const;
template const FlatIndexList* GroundConjunctiveConditionImpl::get_compressed_precondition<NegativeTag, DerivedTag>() const;

const HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag>&
GroundConjunctiveConditionImpl::get_literals() const
{
    return m_preconditions;
}

const GroundNumericConstraintList& GroundConjunctiveConditionImpl::get_numeric_constraints() const { return m_numeric_constraints; }

}
