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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_TO_POSITIVE_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_TO_POSITIVE_NORMAL_FORM_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/base_cached_recurse.hpp"

namespace mimir
{

class ToPositiveNormalFormTransformer : public BaseCachedRecurseTransformer<ToPositiveNormalFormTransformer>
{
private:
    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<ToPositiveNormalFormTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer<ToPositiveNormalFormTransformer>::prepare_impl;
    using BaseCachedRecurseTransformer<ToPositiveNormalFormTransformer>::transform_impl;

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
    void prepare_impl(const ActionImpl& action);

    /* Transform step */

    template<PredicateTag P>
    void transform_conditions(const LiteralList<P>& conditions,
                              const std::unordered_map<Literal<P>, Literal<Derived>>& negative_transformed_duals,
                              LiteralList<P>& ref_transformed_conditions,
                              LiteralList<Derived>& ref_transformed_derived_conditions);

    template<PredicateTag P>
    void compute_duals(const DomainImpl& domain,
                       const LiteralSet<P>& negative_conditions,
                       std::unordered_map<Literal<P>, Literal<Derived>>& out_negative_transformed_duals,
                       PredicateList<Derived>& ref_transformed_derived_predicates);

    std::unordered_map<Literal<Static>, Literal<Derived>> m_negative_static_transformed_duals;
    std::unordered_map<Literal<Fluent>, Literal<Derived>> m_negative_fluent_transformed_duals;
    std::unordered_map<Literal<Derived>, Literal<Derived>> m_negative_derived_transformed_duals;

    EffectConditional transform_impl(const EffectConditionalImpl& effect);
    Action transform_impl(const ActionImpl& action);

    template<PredicateTag P>
    void introduce_axiom_for_dual(const std::unordered_map<Literal<P>, Literal<Derived>>& negative_transformed_duals, AxiomList& ref_axioms);

    Domain transform_impl(const DomainImpl& domain);

public:
    explicit ToPositiveNormalFormTransformer(PDDLRepositories& pddl_repositories);
};

/**
 * Implementations
 */

template<PredicateTag P>
void ToPositiveNormalFormTransformer::transform_conditions(const LiteralList<P>& conditions,
                                                           const std::unordered_map<Literal<P>, Literal<Derived>>& negative_transformed_duals,
                                                           LiteralList<P>& ref_transformed_conditions,
                                                           LiteralList<Derived>& ref_transformed_derived_conditions)
{
    for (const auto& literal : conditions)
    {
        // Skip equality because it is not needed to distinguish between good and bad effect.
        if (literal->is_negated() && literal->get_atom()->get_predicate()->get_name() != "=")
        {
            std::cout << *literal << std::endl;
            const auto& negative_transformed_dual = negative_transformed_duals.at(literal);
            assert(!negative_transformed_dual->is_negated());
            // We need to use the term list from the context, i.e., from the literal of the condition.
            auto transformed_dual = m_pddl_repositories.get_or_create_literal<Derived>(
                false,
                m_pddl_repositories.get_or_create_atom<Derived>(negative_transformed_dual->get_atom()->get_predicate(),
                                                                transform(literal->get_atom()->get_terms())));
            ref_transformed_derived_conditions.push_back(transformed_dual);
        }
        else
        {
            ref_transformed_conditions.push_back(this->transform(*literal));
        }
    }
}

template<PredicateTag P>
void ToPositiveNormalFormTransformer::compute_duals(const DomainImpl& domain,
                                                    const LiteralSet<P>& negative_conditions,
                                                    std::unordered_map<Literal<P>, Literal<Derived>>& out_negative_transformed_duals,
                                                    PredicateList<Derived>& ref_transformed_derived_predicates)
{
    out_negative_transformed_duals.clear();
    for (const auto& negative_literal : negative_conditions)
    {
        // Create dual predicate
        const auto& predicate = negative_literal->get_atom()->get_predicate();
        if (predicate->get_name() == "=")
        {
            // Skip equality because it is not needed to distinguish between good and bad effect.
            continue;
        }

        const auto dual_predicate_name = "not " + predicate->get_name();
        if (domain.get_name_to_predicate<Static>().count(dual_predicate_name)     //
            || domain.get_name_to_predicate<Fluent>().count(dual_predicate_name)  //
            || domain.get_name_to_predicate<Derived>().count(dual_predicate_name))
        {
            throw std::runtime_error("Tried to create dual predicate with name that already exists. Please contact the developers.");
        }

        // transformed_dual_for_negative_literal must use term list
        // that matches the parameters of the introduced derived predicate
        // because we will use the derived predicate parameters as axiom parameters.
        const auto transformed_parameters = predicate->get_parameters();
        const auto transformed_dual_derived_predicate = m_pddl_repositories.get_or_create_predicate<Derived>(dual_predicate_name, transformed_parameters);
        ref_transformed_derived_predicates.push_back(transformed_dual_derived_predicate);
        auto transformed_terms = TermList {};
        for (const auto& variable : transformed_parameters)
        {
            transformed_terms.push_back(m_pddl_repositories.get_or_create_term(variable));
        }
        const auto transformed_atom = m_pddl_repositories.get_or_create_atom(transformed_dual_derived_predicate, transformed_terms);
        // Create dual literal (positive) for negative literal
        const auto transformed_dual_for_negative_literal = m_pddl_repositories.get_or_create_literal(false, transformed_atom);
        out_negative_transformed_duals.emplace(negative_literal, transformed_dual_for_negative_literal);
    }
}

template<PredicateTag P>
void ToPositiveNormalFormTransformer::introduce_axiom_for_dual(const std::unordered_map<Literal<P>, Literal<Derived>>& negative_transformed_duals,
                                                               AxiomList& ref_axioms)
{
    for (const auto& [negative_literal, transformed_dual_positive_literal] : negative_transformed_duals)
    {
        assert(negative_literal->is_negated());
        assert(!transformed_dual_positive_literal->is_negated());

        // transformed_negative_literal must use term list that matches the parameters of derived predicates
        // because we will use the derived predicate parameters as axiom parameters.
        const auto transformed_negative_literal =
            m_pddl_repositories.get_or_create_literal(true,
                                                      m_pddl_repositories.get_or_create_atom(this->transform(*negative_literal->get_atom()->get_predicate()),
                                                                                             transformed_dual_positive_literal->get_atom()->get_terms()));

        const auto& transformed_parameters = transformed_dual_positive_literal->get_atom()->get_predicate()->get_parameters();

        auto transformed_static_conditions = LiteralList<Static> {};
        auto transformed_fluent_conditions = LiteralList<Fluent> {};
        auto transformed_derived_conditions = LiteralList<Derived> {};

        if constexpr (std::is_same_v<P, Static>)
        {
            transformed_static_conditions.push_back(transformed_negative_literal);
        }
        else if constexpr (std::is_same_v<P, Fluent>)
        {
            transformed_fluent_conditions.push_back(transformed_negative_literal);
        }
        else if constexpr (std::is_same_v<P, Derived>)
        {
            transformed_derived_conditions.push_back(transformed_negative_literal);
        }
        else
        {
            static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
        }

        ref_axioms.push_back(this->m_pddl_repositories.get_or_create_axiom(transformed_parameters,
                                                                           transformed_dual_positive_literal,
                                                                           transformed_static_conditions,
                                                                           transformed_fluent_conditions,
                                                                           transformed_derived_conditions));
    }
}

}

#endif
