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

    void prepare_impl(const LiteralImpl<Static>& literal);
    void prepare_impl(const LiteralImpl<Fluent>& literal);
    void prepare_impl(const EffectConditionalImpl& effect);
    void prepare_impl(const EffectUniversalImpl& effect);
    void prepare_impl(const ActionImpl& action);

    /* Transform step */

    template<PredicateCategory P>
    void transform_conditions(const LiteralList<P>& conditions,
                              const std::unordered_map<Literal<P>, Literal<P>>& negative_duals,
                              LiteralList<P>& out_transformed_conditions);

    template<PredicateCategory P>
    void compute_duals(const DomainImpl& domain,
                       const LiteralSet<P>& negative_conditions,
                       const std::unordered_map<Atom<P>, Literal<P>>& positive_literals_by_atom,
                       std::unordered_map<Literal<P>, Literal<P>>& out_positive_duals,
                       std::unordered_map<Literal<P>, Literal<P>>& out_negative_duals,
                       std::unordered_map<Predicate<P>, Predicate<P>>& ref_dual_predicates);

    std::unordered_map<Predicate<Static>, Predicate<Static>> m_static_dual_predicates;
    std::unordered_map<Predicate<Fluent>, Predicate<Fluent>> m_fluent_dual_predicates;

    std::unordered_map<Literal<Static>, Literal<Static>> m_positive_static_duals;
    std::unordered_map<Literal<Static>, Literal<Static>> m_negative_static_duals;
    std::unordered_map<Literal<Fluent>, Literal<Fluent>> m_positive_fluent_duals;
    std::unordered_map<Literal<Fluent>, Literal<Fluent>> m_negative_fluent_duals;

    Action transform_impl(const ActionImpl& action);
    Domain transform_impl(const DomainImpl& domain);
    Problem transform_impl(const ProblemImpl& problem);

public:
    explicit PositiveNormalFormTransformer(PDDLFactories& pddl_factories);
};

/**
 * Implementations
 */

template<PredicateCategory P>
void PositiveNormalFormTransformer::transform_conditions(const LiteralList<P>& conditions,
                                                         const std::unordered_map<Literal<P>, Literal<P>>& negative_duals,
                                                         LiteralList<P>& out_transformed_conditions)
{
    out_transformed_conditions.clear();
    for (const auto& literal : conditions)
    {
        if (negative_duals.count(literal))
        {
            out_transformed_conditions.push_back(negative_duals.at(literal));
        }
        {
            out_transformed_conditions.push_back(transform(*literal));
        }
    }
}

template<PredicateCategory P>
void PositiveNormalFormTransformer::compute_duals(const DomainImpl& domain,
                                                  const LiteralSet<P>& negative_conditions,
                                                  const std::unordered_map<Atom<P>, Literal<P>>& positive_literals_by_atom,
                                                  std::unordered_map<Literal<P>, Literal<P>>& out_positive_duals,
                                                  std::unordered_map<Literal<P>, Literal<P>>& out_negative_duals,
                                                  std::unordered_map<Predicate<P>, Predicate<P>>& out_dual_predicates)
{
    out_positive_duals.clear();
    out_negative_duals.clear();
    out_dual_predicates.clear();
    for (const auto& negative_literal : negative_conditions)
    {
        // Create dual predicate
        const auto& predicate = negative_literal->get_atom()->get_predicate();
        const auto dual_predicate_name = "not " + predicate->get_name();
        if (domain.get_name_to_predicate<P>().count(dual_predicate_name))
        {
            throw std::runtime_error("Tried to create dual predicate with name that already exists. Please contact the developers.");
        }
        const auto dual_predicate = m_pddl_factories.get_or_create_predicate<P>(dual_predicate_name, predicate->get_parameters());
        out_dual_predicates.emplace(predicate, dual_predicate);

        // Create dual literal (negative) for positive literal
        const auto positive_literal = positive_literals_by_atom.at(negative_literal->get_atom());
        const auto dual_for_positive_literal =
            m_pddl_factories.get_or_create_literal(true, m_pddl_factories.get_or_create_atom(dual_predicate, positive_literal->get_atom()->get_terms()));
        out_positive_duals.emplace(positive_literal, dual_for_positive_literal);

        // Create dual literal (positive) for negative literal
        const auto dual_for_negative_literal =
            m_pddl_factories.get_or_create_literal(false, m_pddl_factories.get_or_create_atom(dual_predicate, negative_literal->get_atom()->get_terms()));
        out_negative_duals.emplace(negative_literal, dual_for_negative_literal);
    }
}

}

#endif