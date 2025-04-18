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
    Index m_index;
    Action m_action;
    ObjectList m_objects;
    GroundConjunctiveCondition m_conjunctive_precondition;
    GroundConjunctiveEffect m_conjunctive_effect;
    GroundConditionalEffectList m_conditional_effects;

    GroundActionImpl(Index index,
                     Action action,
                     ObjectList objects,
                     GroundConjunctiveCondition conjunctive_precondition,
                     GroundConjunctiveEffect conjunctive_effect,
                     GroundConditionalEffectList conditional_effects);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    using FormalismEntity = void;

    // moveable but not copyable
    GroundActionImpl(const GroundActionImpl& other) = delete;
    GroundActionImpl& operator=(const GroundActionImpl& other) = delete;
    GroundActionImpl(GroundActionImpl&& other) = default;
    GroundActionImpl& operator=(GroundActionImpl&& other) = default;

    /// @brief `FullFormatterTag` is used to dispatch the operator<< overload that prints the entire action.
    struct FullFormatterTag
    {
    };
    /// @brief `PlanFormatterTag` is used to dispatch the operator<< overload that prints the action as it appears in a plan.
    struct PlanFormatterTag
    {
    };

    Index get_index() const;
    Action get_action() const;
    const ObjectList& get_objects() const;
    GroundConjunctiveCondition get_conjunctive_condition() const;
    GroundConjunctiveEffect get_conjunctive_effect() const;
    const GroundConditionalEffectList& get_conditional_effects() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the lifted schema index and the binding because they imply the rest.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_action(), std::cref(get_objects())); }
};

/// @brief STL does not define operator== for std::span.
inline bool operator==(const std::span<const GroundAction>& lhs, const std::span<const GroundAction>& rhs)
{
    return (lhs.data() == rhs.data()) && (lhs.size() == rhs.size());
}
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
