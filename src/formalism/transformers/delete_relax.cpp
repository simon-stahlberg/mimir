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

Action DeleteRelaxTransformer::transform_impl(const ActionImpl& action)
{
    auto parameters = this->transform(action.get_parameters());
    auto conditions = this->transform(action.get_conditions());

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

    return this->m_pddl_factories.get_or_create_action(action.get_name(), parameters, conditions, effects, this->transform(*action.get_function_expression()));
}

Problem DeleteRelaxTransformer::run_impl(const ProblemImpl& problem) { return this->transform(problem); }

DeleteRelaxTransformer::DeleteRelaxTransformer(PDDLFactories& pddl_factories) : BaseTransformer<DeleteRelaxTransformer>(pddl_factories) {}

}
