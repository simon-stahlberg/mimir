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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_DELETE_RELAX_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_DELETE_RELAX_HPP_

#include "mimir/formalism/transformers/base_cached_recurse.hpp"

namespace mimir
{

class DeleteRelaxTransformer : public BaseCachedRecurseTransformer<DeleteRelaxTransformer>
{
private:
    bool m_remove_useless_actions_and_axioms;

    // There can be a one to many relationship between relaxed and unrelaxed.
    std::unordered_map<Action, ActionList> m_delete_to_normal_actions;
    std::unordered_map<Axiom, AxiomList> m_delete_to_normal_axioms;

    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<DeleteRelaxTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer<DeleteRelaxTransformer>::prepare_impl;
    using BaseCachedRecurseTransformer<DeleteRelaxTransformer>::transform_impl;

    template<PredicateCategory P>
    LiteralList<P> transform_impl(const LiteralList<P>& literals);
    EffectSimpleList transform_impl(const EffectSimpleList& effects);
    EffectConditionalList transform_impl(const EffectConditionalList& effects);
    EffectUniversalList transform_impl(const EffectUniversalList& effects);
    ActionList transform_impl(const ActionList& actions);
    AxiomList transform_impl(const AxiomList& axioms);

    template<PredicateCategory P>
    Literal<P> transform_impl(const LiteralImpl<P>& literal);
    EffectSimple transform_impl(const EffectSimpleImpl& effect);
    EffectConditional transform_impl(const EffectConditionalImpl& effect);
    EffectUniversal transform_impl(const EffectUniversalImpl& effect);
    Action transform_impl(const ActionImpl& action);
    Axiom transform_impl(const AxiomImpl& axiom);
    Domain transform_impl(const DomainImpl& domain);

    Problem run_impl(const ProblemImpl& problem);

public:
    /// @brief
    /// @param pddl_factories
    /// @param keep_useless_actions_and_axioms flag to indicate whether actions
    /// and axioms with empty effects should be kept.
    /// We need those in the grounded successor generator.
    /// However, we do not need them when computing relaxed plans.
    DeleteRelaxTransformer(PDDLFactories& pddl_factories, bool remove_useless_actions_and_axioms = true);

    const ActionList& get_unrelaxed_actions(Action action) const;

    const AxiomList& get_unrelaxed_axioms(Axiom axiom) const;
};

/**
 * Implementations
 */

template<PredicateCategory P>
LiteralList<P> DeleteRelaxTransformer::transform_impl(const LiteralList<P>& literals)
{
    auto positive_literals = LiteralList<P> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->transform(*literal);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

template<PredicateCategory P>
Literal<P> DeleteRelaxTransformer::transform_impl(const LiteralImpl<P>& literal)
{
    if (literal.is_negated())
    {
        return nullptr;
    }

    const auto atom = this->transform(*literal.get_atom());

    return this->m_pddl_factories.get_or_create_literal(false, atom);
}

}

#endif