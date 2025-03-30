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

#ifndef MIMIR_FORMALISM_GROUND_ACTION_HPP_
#define MIMIR_FORMALISM_GROUND_ACTION_HPP_

#include "mimir/buffering/unordered_set.h"
#include "mimir/buffering/vector.h"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_conjunctive_condition.hpp"
#include "mimir/formalism/ground_effects.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{
class GroundActionImpl
{
private:
    Index m_index = Index(0);
    Index m_action_index = Index(0);
    FlatIndexList m_object_indices = FlatIndexList();
    GroundConjunctiveCondition m_conjunctive_precondition = GroundConjunctiveCondition();
    GroundConjunctiveEffect m_conjunctive_effect = GroundConjunctiveEffect();
    GroundEffectConditionalList m_conditional_effects = GroundEffectConditionalList();

public:
    using FormalismEntity = void;

    /// @brief `FullFormatterTag` is used to dispatch the operator<< overload that prints the entire action.
    struct FullFormatterTag
    {
    };
    /// @brief `PlanFormatterTag` is used to dispatch the operator<< overload that prints the action as it appears in a plan.
    struct PlanFormatterTag
    {
    };

    Index& get_index();
    Index& get_action_index();
    FlatIndexList& get_objects();

    Index get_index() const;
    Index get_action_index() const;
    const FlatIndexList& get_object_indices() const;

    /* Conjunctive part */
    GroundConjunctiveCondition& get_conjunctive_condition();
    const GroundConjunctiveCondition& get_conjunctive_condition() const;
    GroundConjunctiveEffect& get_conjunctive_effect();
    const GroundConjunctiveEffect& get_conjunctive_effect() const;
    /* Conditional effects */
    GroundEffectConditionalList& get_conditional_effects();
    const GroundEffectConditionalList& get_conditional_effects() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the lifted schema index and the binding because they imply the rest.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_action_index(), std::cref(get_object_indices())); }

    auto cista_members() noexcept
    {
        return std::tie(m_index, m_action_index, m_object_indices, m_conjunctive_precondition, m_conjunctive_effect, m_conditional_effects);
    }
};

/// @brief STL does not define operator== for std::span.
inline bool operator==(const std::span<const GroundAction>& lhs, const std::span<const GroundAction>& rhs)
{
    return (lhs.data() == rhs.data()) && (lhs.size() == rhs.size());
}

/**
 * Mimir types
 */

using GroundActionImplSet = mimir::buffering::UnorderedSet<GroundActionImpl>;

/**
 * Pretty printing
 */

}

namespace mimir
{
template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<formalism::GroundAction, const formalism::ProblemImpl&, formalism::GroundActionImpl::FullFormatterTag>& data);

template<>
std::ostream& operator<<(std::ostream& os,
                         const std::tuple<formalism::GroundAction, const formalism::ProblemImpl&, formalism::GroundActionImpl::PlanFormatterTag>& data);

}

#endif
