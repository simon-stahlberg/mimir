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

#include "mimir/common/concepts.hpp"
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
    LiteralSet<Derived> m_negative_derived_conditions;
    LiteralSet<Derived> m_derived_literals;

    void prepare_impl(const LiteralImpl<Static>& literal);
    void prepare_impl(const LiteralImpl<Fluent>& literal);
    void prepare_impl(const LiteralImpl<Derived>& literal);
    void prepare_impl(const EffectConditionalImpl& effect);
    void prepare_impl(const EffectUniversalImpl& effect);
    void prepare_impl(const ActionImpl& action);

    /* Transform step */

    template<PredicateCategory P>
    void transform_conditions(const LiteralList<P>& conditions,
                              const std::unordered_map<Literal<P>, Literal<Derived>>& negative_duals,
                              LiteralList<P>& ref_transformed_conditions,
                              LiteralList<Derived>& ref_transformed_derived_conditions);

    template<PredicateCategory P>
    void compute_duals(const DomainImpl& domain,
                       const LiteralSet<P>& negative_conditions,
                       std::unordered_map<Literal<P>, Literal<Derived>>& out_negative_duals,
                       PredicateList<Derived>& ref_derived_predicates);

    std::unordered_map<Literal<Static>, Literal<Derived>> m_negative_static_duals;
    std::unordered_map<Literal<Fluent>, Literal<Derived>> m_negative_fluent_duals;
    std::unordered_map<Literal<Derived>, Literal<Derived>> m_negative_derived_duals;

    EffectConditional transform_impl(const EffectConditionalImpl& effect);
    EffectUniversal transform_impl(const EffectUniversalImpl& effect);
    Action transform_impl(const ActionImpl& action);

    template<PredicateCategory P>
    void introduce_axiom_for_dual(const std::unordered_map<Literal<P>, Literal<Derived>>& negative_duals, AxiomList& ref_axioms);

    Domain transform_impl(const DomainImpl& domain);

public:
    explicit PositiveNormalFormTransformer(PDDLFactories& pddl_factories);
};

/**
 * Implementations
 */

template<PredicateCategory P>
void PositiveNormalFormTransformer::transform_conditions(const LiteralList<P>& conditions,
                                                         const std::unordered_map<Literal<P>, Literal<Derived>>& negative_duals,
                                                         LiteralList<P>& ref_transformed_conditions,
                                                         LiteralList<Derived>& ref_transformed_derived_conditions)
{
    for (const auto& literal : conditions)
    {
        if (negative_duals.count(literal))
        {
            assert(literal->is_negated());
            ref_transformed_derived_conditions.push_back(negative_duals.at(literal));
        }
        {
            ref_transformed_conditions.push_back(this->transform(*literal));
        }
    }
}

template<PredicateCategory P>
void PositiveNormalFormTransformer::compute_duals(const DomainImpl& domain,
                                                  const LiteralSet<P>& negative_conditions,
                                                  std::unordered_map<Literal<P>, Literal<Derived>>& out_negative_duals,
                                                  PredicateList<Derived>& ref_derived_predicates)
{
    out_negative_duals.clear();
    for (const auto& negative_literal : negative_conditions)
    {
        // Create dual predicate
        const auto& predicate = negative_literal->get_atom()->get_predicate();
        const auto dual_predicate_name = "not " + predicate->get_name();
        if (domain.get_name_to_predicate<Static>().count(dual_predicate_name)     //
            || domain.get_name_to_predicate<Fluent>().count(dual_predicate_name)  //
            || domain.get_name_to_predicate<Derived>().count(dual_predicate_name))
        {
            throw std::runtime_error("Tried to create dual predicate with name that already exists. Please contact the developers.");
        }
        const auto dual_derived_predicate = m_pddl_factories.get_or_create_predicate<Derived>(dual_predicate_name, predicate->get_parameters());
        ref_derived_predicates.push_back(dual_derived_predicate);

        // Create dual literal (positive) for negative literal
        const auto dual_for_negative_literal =
            m_pddl_factories.get_or_create_literal(false,
                                                   m_pddl_factories.get_or_create_atom(dual_derived_predicate, negative_literal->get_atom()->get_terms()));
        out_negative_duals.emplace(negative_literal, dual_for_negative_literal);
    }
}

template<PredicateCategory P>
void PositiveNormalFormTransformer::introduce_axiom_for_dual(const std::unordered_map<Literal<P>, Literal<Derived>>& negative_duals, AxiomList& ref_axioms)
{
    for (const auto& [negative_literal, dual_positive_literal] : negative_duals)
    {
        assert(negative_literal->is_negated());
        assert(!dual_positive_literal->is_negated());
        assert(negative_literal->get_atom()->get_predicate()->get_parameters() == dual_positive_literal->get_atom()->get_predicate()->get_parameters());
        const auto& parameters = negative_literal->get_atom()->get_predicate()->get_parameters();

        auto static_conditions = LiteralList<Static> {};
        auto fluent_conditions = LiteralList<Fluent> {};
        auto derived_conditions = LiteralList<Derived> {};

        if constexpr (std::is_same_v<P, Static>)
        {
            static_conditions.push_back(this->transform(*negative_literal));
        }
        else if constexpr (std::is_same_v<P, Fluent>)
        {
            fluent_conditions.push_back(this->transform(*negative_literal));
        }
        else if constexpr (std::is_same_v<P, Derived>)
        {
            derived_conditions.push_back(this->transform(*negative_literal));
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
        }

        ref_axioms.push_back(
            this->m_pddl_factories.get_or_create_axiom(parameters, dual_positive_literal, static_conditions, fluent_conditions, derived_conditions));
    }
}

}

#endif