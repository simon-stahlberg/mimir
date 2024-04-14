/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

/**
 * Translate to effect normal form:
 * https://gki.informatik.uni-freiburg.de/teaching/ss05/aip/aip02.pdf
 */
class ToENFTranslator : public BaseTranslator<ToENFTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<ToENFTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    loki::Effect translate_impl(const loki::EffectAndImpl& effect);
    loki::Effect translate_impl(const loki::EffectConditionalForallImpl& effect);
    loki::Effect translate_impl(const loki::EffectConditionalWhenImpl& effect);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit ToENFTranslator(loki::PDDLFactories& pddl_factories);
};
}

#endif
