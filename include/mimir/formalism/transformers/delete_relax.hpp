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
    // There can be a one to many relationship between relaxed and unrelaxed.
    std::unordered_map<Action, ActionList> m_delete_to_normal_actions;
    std::unordered_map<Axiom, AxiomList> m_delete_to_normal_axioms;

    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<DeleteRelaxTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer<DeleteRelaxTransformer>::prepare_impl;
    using BaseCachedRecurseTransformer<DeleteRelaxTransformer>::transform_impl;

    template<PredicateTag P>
    LiteralList<P> transform_impl(const LiteralList<P>& literals);
    ConditionalEffectList transform_impl(const ConditionalEffectList& effects);
    ActionList transform_impl(const ActionList& actions);
    AxiomList transform_impl(const AxiomList& axioms);

    template<PredicateTag P>
    Literal<P> transform_impl(Literal<P> literal);
    ConjunctiveCondition transform_impl(ConjunctiveCondition condition);
    ConjunctiveEffect transform_impl(ConjunctiveEffect effect);
    ConditionalEffect transform_impl(ConditionalEffect effect);
    Action transform_impl(Action action);
    Axiom transform_impl(Axiom axiom);

    Problem run_impl(Problem problem);

public:
    /// @brief
    /// @param pddl_repositories
    /// @param keep_useless_actions_and_axioms flag to indicate whether actions
    /// and axioms with empty effects should be kept.
    /// We need those in the grounded successor generator.
    /// However, we do not need them when computing relaxed plans.
    explicit DeleteRelaxTransformer(PDDLRepositories& pddl_repositories);

    const ActionList& get_unrelaxed_actions(Action action) const;

    const AxiomList& get_unrelaxed_axioms(Axiom axiom) const;
};

}

#endif