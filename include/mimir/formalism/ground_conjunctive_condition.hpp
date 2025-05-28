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

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{
class GroundConjunctiveConditionImpl
{
private:
    Index m_index;
    HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> m_preconditions;
    GroundNumericConstraintList m_numeric_constraints;

    GroundConjunctiveConditionImpl(Index index,
                                   HanaContainer<HanaContainer<const FlatIndexList*, StaticTag, FluentTag, DerivedTag>, PositiveTag, NegativeTag> preconditions,
                                   GroundNumericConstraintList numeric_constraints);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    GroundConjunctiveConditionImpl(const GroundConjunctiveConditionImpl& other) = delete;
    GroundConjunctiveConditionImpl& operator=(const GroundConjunctiveConditionImpl& other) = delete;
    GroundConjunctiveConditionImpl(GroundConjunctiveConditionImpl&& other) = default;
    GroundConjunctiveConditionImpl& operator=(GroundConjunctiveConditionImpl&& other) = default;

    Index get_index() const;

    /**
     * Ranges
     * TODO: add other generic ranges using ranges::views::concat
     */

    template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
    auto get_precondition() const;

    /**
     * Accessors
     * TODO: add other generic accessors over other template dimensions
     */

    template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
    const FlatIndexList* get_compressed_precondition() const;

    template<IsStaticOrFluentOrDerivedTag... Ps>
    auto get_hana_compressed_precondition() const;

    template<IsStaticOrFluentOrDerivedTag... Ps>
    size_t get_num_preconditions() const;

    const GroundNumericConstraintList& get_numeric_constraints() const;

    auto identifying_members() const
    {
        return std::tuple(get_compressed_precondition<PositiveTag, StaticTag>(),
                          get_compressed_precondition<NegativeTag, StaticTag>(),
                          get_compressed_precondition<PositiveTag, FluentTag>(),
                          get_compressed_precondition<NegativeTag, FluentTag>(),
                          get_compressed_precondition<PositiveTag, DerivedTag>(),
                          get_compressed_precondition<NegativeTag, DerivedTag>(),
                          get_numeric_constraints());
    }
};

/**
 * Implementations
 */

template<IsPolarity R, IsStaticOrFluentOrDerivedTag P>
auto GroundConjunctiveConditionImpl::get_precondition() const
{
    return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {})->compressed_range();
}

template<IsStaticOrFluentOrDerivedTag... Ps>
auto GroundConjunctiveConditionImpl::get_hana_compressed_precondition() const
{
    return boost::hana::make_map(
        boost::hana::make_pair(boost::hana::type<PositiveTag> {},
                               boost::hana::make_map(boost::hana::make_pair(
                                   boost::hana::type<Ps> {},
                                   boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<PositiveTag> {}), boost::hana::type<Ps> {}))...)),
        boost::hana::make_pair(
            boost::hana::type<NegativeTag> {},
            boost::hana::make_map(boost::hana::make_pair(
                boost::hana::type<Ps> {},
                boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<NegativeTag> {}), boost::hana::type<Ps> {}))...)));
}

template<IsStaticOrFluentOrDerivedTag... Ps>
size_t GroundConjunctiveConditionImpl::get_num_preconditions() const
{
    auto result = size_t(0);
    boost::hana::for_each(get_hana_compressed_precondition<Ps...>(),
                          [this, &result](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              boost::hana::for_each(second,
                                                    [this, &result](auto&& pair2)
                                                    {
                                                        const auto second2 = boost::hana::second(pair2);
                                                        result += second2->size();
                                                    });
                          });
    return result;
}

}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConjunctiveCondition, const formalism::ProblemImpl&>& data);
}

#endif
