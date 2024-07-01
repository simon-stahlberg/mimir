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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSFORMERS_POSITIVE_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_POSITIVE_NORMAL_FORM_HPP_

#include "mimir/formalism/transformers/base_cached_recurse.hpp"

namespace mimir
{

class PositiveNormalFormTransformer : public BaseCachedRecurseTransformer<PositiveNormalFormTransformer>
{
private:
    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<PositiveNormalFormTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer<PositiveNormalFormTransformer>::prepare_impl;
    using BaseCachedRecurseTransformer<PositiveNormalFormTransformer>::transform_impl;

    /* Prepare step */

    LiteralSet<Static> m_negative_static_conditions;
    LiteralSet<Static> m_static_literals;
    LiteralSet<Fluent> m_negative_fluent_conditions;
    LiteralSet<Fluent> m_fluent_literals;

    void prepare_impl(const LiteralImpl<Fluent>& literal);
    void prepare_impl(const EffectConditionalImpl& effect);
    void prepare_impl(const EffectUniversalImpl& effect);
    void prepare_impl(const ActionImpl& action);

    /* Transform step */

    template<PredicateCategory P>
    void transform_conditions(const LiteralList<P>& conditions,
                              const std::unordered_map<Literal<P>, Literal<P>>& negative_duals,
                              LiteralList<P>& out_transformed_conditions);

    std::unordered_map<Literal<Fluent>, Literal<Fluent>> m_positive_duals;
    std::unordered_map<Literal<Fluent>, Literal<Fluent>> m_negative_duals;

    Action transform_impl(const ActionImpl& action);
    Domain transform_impl(const DomainImpl& domain);

public:
    explicit PositiveNormalFormTransformer(PDDLFactories& pddl_factories);
};

template<PredicateCategory P>
void PositiveNormalFormTransformer::transform_conditions(const LiteralList<P>& conditions,
                                                         const std::unordered_map<Literal<P>, Literal<P>>& negative_duals,
                                                         LiteralList<P>& out_transformed_conditions)
{
    out_transformed_conditions.clear();
    for (const auto& literal : conditions)
    {
        if (m_negative_duals.count(literal))
        {
            out_transformed_conditions.push_back(this->m_negative_duals.at(literal));
        }
        {
            out_transformed_conditions.push_back(transform(*literal));
        }
    }
}
}

#endif