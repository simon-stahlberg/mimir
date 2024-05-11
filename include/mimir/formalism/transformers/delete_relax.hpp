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

/**
 * Introduce an axiom for complicated goals, i.e., goals that are not conjunctions of literals.
 *
 * TODO: update static and fluent conditions because they might change on the delete relaxed problem.
 */
class DeleteRelaxTransformer : public BaseCachedRecurseTransformer<DeleteRelaxTransformer>
{
private:
    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<DeleteRelaxTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer::prepare_impl;
    using BaseCachedRecurseTransformer::transform_impl;

    std::unordered_map<Action, Action> m_delete_to_normal_action;
    std::unordered_map<Axiom, Axiom> m_delete_to_normal_axiom;

    Action transform_impl(const ActionImpl& action);
    Axiom transform_impl(const AxiomImpl& axiom);
    Domain transform_impl(const DomainImpl& domain);

    Problem run_impl(const ProblemImpl& problem);

public:
    explicit DeleteRelaxTransformer(PDDLFactories& pddl_factories);

    Action get_unrelaxed_action(Action action) const;

    Axiom get_unrelaxed_axiom(Axiom axiom) const;
};
}

#endif