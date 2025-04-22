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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_IMPL_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_IMPL_HPP_

#include "mimir/common/debug.hpp"
#include "mimir/datasets/declarations.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/general_policies/declarations.hpp"
#include "mimir/languages/general_policies/general_policy_decl.hpp"
#include "mimir/languages/general_policies/rule.hpp"

#include <stack>

namespace mimir::languages::general_policies
{
template<std::ranges::forward_range VertexIndices>
    requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
SolvabilityStatus
GeneralPolicyImpl::solves(const datasets::StateSpace& state_space, const VertexIndices& vertices, dl::DenotationRepositories& denotation_repositories) const
{
    auto visited_v_idxs = graphs::VertexIndexSet {};

    for (const auto& v_idx : vertices)
    {
        const auto reason = solves(state_space, v_idx, denotation_repositories, visited_v_idxs);

        if (reason == SolvabilityStatus::CYCLIC || reason == SolvabilityStatus::UNSOLVABLE)
        {
            return reason;
        }
    }

    return SolvabilityStatus::SOLVED;
}

template<std::ranges::forward_range VertexIndices>
    requires std::same_as<std::ranges::range_value_t<VertexIndices>, graphs::VertexIndex>
SolvabilityStatus GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                            const VertexIndices& vertices,
                                            dl::DenotationRepositories& denotation_repositories) const
{
    auto visited_v_idxs = graphs::VertexIndexSet {};

    for (const auto& v_idx : vertices)
    {
        const auto reason = solves(generalized_state_space, v_idx, denotation_repositories, visited_v_idxs);

        if (reason == SolvabilityStatus::CYCLIC || reason == SolvabilityStatus::UNSOLVABLE)
        {
            return reason;
        }
    }

    return SolvabilityStatus::SOLVED;
}

}

#endif