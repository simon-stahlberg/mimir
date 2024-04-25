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

#include "mimir/formalism/transformers/delete_relax.hpp"

namespace mimir
{

static LiteralList filter_positive_literals(const LiteralList& literals)
{
    auto positive_literals = LiteralList {};
    for (const auto& literal : literals)
    {
        if (!literal->is_negated())
        {
            positive_literals.push_back(literal);
        }
    }
    return positive_literals;
}

Action DeleteRelaxTransformer::transform_impl(const ActionImpl& action)
{
    auto parameters = this->transform(action.get_parameters());
    auto conditions = filter_positive_literals(this->transform(action.get_conditions()));
    auto static_conditions = filter_positive_literals(this->transform(action.get_static_conditions()));
    auto fluent_conditions = filter_positive_literals(this->transform(action.get_fluent_conditions()));

    // Remove negative effects.
    auto effects = EffectList {};
    for (const auto& effect : this->transform(action.get_effects()))
    {
        if (!effect->get_effect()->is_negated())
        {
            effects.push_back(effect);
        }
    }
    effects.shrink_to_fit();

    auto delete_relaxed_action = this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                                             parameters,
                                                                             conditions,
                                                                             static_conditions,
                                                                             fluent_conditions,
                                                                             effects,
                                                                             this->transform(*action.get_function_expression()));

    m_delete_to_normal_action.emplace(delete_relaxed_action, &action);

    return delete_relaxed_action;
}

Problem DeleteRelaxTransformer::run_impl(const ProblemImpl& problem) { return this->transform(problem); }

DeleteRelaxTransformer::DeleteRelaxTransformer(PDDLFactories& pddl_factories) : BaseCachedRecurseTransformer<DeleteRelaxTransformer>(pddl_factories) {}

Action DeleteRelaxTransformer::get_unrelaxed_action(Action action) const { return m_delete_to_normal_action.at(action); }

}
