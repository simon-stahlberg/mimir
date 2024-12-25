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

#ifndef MIMIR_FORMALISM_ACTION_HPP_
#define MIMIR_FORMALISM_ACTION_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ActionImpl
{
private:
    Index m_index;
    std::string m_name;
    size_t m_original_arity;
    ExistentiallyQuantifiedConjunctiveCondition m_precondition;
    EffectStrips m_strips_effect;
    EffectConditionalList m_conditional_effects;

    // Below: add additional members if needed and initialize them in the constructor

    ActionImpl(Index index,
               std::string name,
               size_t original_arity,
               ExistentiallyQuantifiedConjunctiveCondition precondition,
               EffectStrips strips_effect,
               EffectConditionalList conditional_effects);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ActionImpl(const ActionImpl& other) = delete;
    ActionImpl& operator=(const ActionImpl& other) = delete;
    ActionImpl(ActionImpl&& other) = default;
    ActionImpl& operator=(ActionImpl&& other) = default;

    Index get_index() const;
    const std::string& get_name() const;
    size_t get_original_arity() const;
    const VariableList& get_parameters() const;
    const ExistentiallyQuantifiedConjunctiveCondition& get_precondition() const;
    const EffectStrips& get_strips_effect() const;
    const EffectConditionalList& get_conditional_effects() const;

    size_t get_arity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifiable_members() const
    {
        return std::forward_as_tuple(std::as_const(m_name),
                                     std::as_const(m_original_arity),
                                     std::as_const(m_precondition),
                                     std::as_const(m_strips_effect),
                                     std::as_const(m_conditional_effects));
    }
};

/**
 * Pretty printing
 */

extern std::ostream& operator<<(std::ostream& out, const ActionImpl& element);

extern std::ostream& operator<<(std::ostream& out, Action element);

}

#endif
