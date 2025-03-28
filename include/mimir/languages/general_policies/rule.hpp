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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_RULE_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_RULE_HPP_

#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
class RuleImpl
{
private:
    Index m_index;
    ConditionList m_conditions;
    EffectList m_effects;

    RuleImpl(Index index, ConditionList conditions, EffectList effects);

    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    bool evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    const ConditionList& get_conditions() const;
    const EffectList& get_effects() const;

    auto identifying_members() const { return std::tuple(std::cref(get_conditions()), std::cref(get_effects())); }
};
}

#endif