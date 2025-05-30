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

#ifndef MIMIR_SEARCH_HEURISTICS_RPG_STRUCTURES_HPP_
#define MIMIR_SEARCH_HEURISTICS_RPG_STRUCTURES_HPP_

#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/heuristics/rpg/annotations.hpp"

namespace mimir::search::rpg
{
using ConditionsContainer =
    HanaContainer<HanaContainer<const FlatIndexList*, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag>;

class Action
{
public:
    Action(Index index,
           formalism::GroundAction unrelaxed_action,
           ConditionsContainer preconditions,
           ConditionsContainer conditional_preconditions,
           size_t num_preconditions,
           Index fluent_effect,
           bool polarity) :
        m_index(index),
        m_unrelaxed_action(unrelaxed_action),
        m_preconditions(preconditions),
        m_conditional_preconditions(conditional_preconditions),
        m_num_preconditions(num_preconditions),
        m_fluent_effect(fluent_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    formalism::GroundAction get_unrelaxed_action() const { return m_unrelaxed_action; }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_conditional_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_conditional_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_effect() const { return m_fluent_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    formalism::GroundAction m_unrelaxed_action;
    ConditionsContainer m_preconditions;
    ConditionsContainer m_conditional_preconditions;
    size_t m_num_preconditions;
    Index m_fluent_effect;
    bool m_polarity;
};
using ActionList = std::vector<Action>;

class Axiom
{
public:
    Axiom(Index index, ConditionsContainer preconditions, size_t num_preconditions, Index derived_effect, bool polarity) :
        m_index(index),
        m_num_preconditions(num_preconditions),
        m_derived_effect(derived_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_effect() const { return m_derived_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    ConditionsContainer m_preconditions;
    size_t m_num_preconditions;
    Index m_derived_effect;
    bool m_polarity;
};
using AxiomList = std::vector<Axiom>;

template<typename T>
concept IsStructure = requires(T a) {
    { a.get_index() } -> std::convertible_to<Index>;
    { a.get_num_preconditions() } -> std::convertible_to<size_t>;
    { a.get_effect() } -> std::convertible_to<Index>;
    { a.get_polarity() } -> std::convertible_to<bool>;
};

using StructuresContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Action>, ActionList>, boost::hana::pair<boost::hana::type<Axiom>, AxiomList>>;

template<IsStructure S>
inline auto& get(StructuresContainer& container)
{
    return boost::hana::at_key(container, boost::hana::type<S> {});
}

using StructuresAnnotations = HanaContainer<AnnotationsList<DiscreteCost, size_t>, Action, Axiom>;

template<IsStructure S>
inline auto& get(StructuresAnnotations& annotations)
{
    return boost::hana::at_key(annotations, boost::hana::type<S> {});
}

}

#endif
