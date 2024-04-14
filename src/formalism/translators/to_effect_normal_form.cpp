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

#include "mimir/formalism/translators/to_effect_normal_form.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

loki::Effect ToENFTranslator::translate_impl(const loki::EffectAndImpl& effect) {}
loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect) {}
/**
 * 1. phi > (e1 and e2)  =>  (phi > e1) and (phi > e2)
 * 2. phi > (psi > e)    =>  (phi and psi) > e
 */
loki::Effect ToENFTranslator::translate_impl(const loki::EffectConditionalWhenImpl& effect) {}

loki::Problem ToENFTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

ToENFTranslator::ToENFTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
