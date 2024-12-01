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

#ifndef MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TO_EFFECT_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base_cached_recurse.hpp"

namespace mimir
{

/**
 * Translate to effect normal form in bottom-up manner using the following rules:
 *
 * 1. e and (phi > e)          =>  e
 * 2. e1 and (e2 and e3)       =>  e1 and e2 and e3
 * 3. forall(vars1, forall(vars2, e))    =>  forall(vars1+vars2, e)
 * 4. forall(vars, (e1 and e2))          => forall(vars, e1) and forall(vars, e2)
 * 5. phi > (psi > e)    =>  (phi and psi) > e
 * 6. phi > (e1 and e2)  =>  (phi > e1) and (phi > e2)
 * 7. phi > forall(vars, e)  => forall(vars, phi > e)
 * 8. exists(vars, phi) > e  => forall(vars, phi > e)
 * 9. phi and (psi and chi)  =>  phi and psi and chi
 *
 * We stabilize the result before returning it since applying a rule might allow for other rules.
 */
class ToENFTranslator : public BaseCachedRecurseTranslator<ToENFTranslator>
{
private:
    /* Implement BaseCachedRecurseTranslator interface. */
    friend class BaseCachedRecurseTranslator<ToENFTranslator>;

    // Provide default implementations
    using BaseCachedRecurseTranslator::prepare_impl;
    using BaseCachedRecurseTranslator::translate_impl;

    /**
     * 1. e and (phi > e)          =>  e
     * 2. e1 and (e2 and e3)       =>  e1 and e2 and e3
     */
    loki::Effect translate_impl(const loki::EffectAndImpl& effect);
    /**
     * 3. forall(vars1, forall(vars2, e))    =>  forall(vars1+vars2, e)
     * 4. forall(vars, (e1 and e2))          => forall(vars, e1) and forall(vars, e2)
     */
    loki::Effect translate_impl(const loki::EffectCompositeForallImpl& effect);
    /**
     * 5. phi > (psi > e)    =>  (phi and psi) > e
     * 6. phi > (e1 and e2)  =>  (phi > e1) and (phi > e2)
     * 7. phi > forall(vars, e)  => forall(vars, phi > e)
     * 8. exists(vars, phi) > e  => forall(vars, phi > e)
     */
    loki::Effect translate_impl(const loki::EffectCompositeWhenImpl& effect);
    /**
     * Flatten conjunctions.
     *
     * 9. phi and (psi and chi)  =>  phi and psi and chi
     */
    loki::Condition translate_impl(const loki::ConditionAndImpl& condition);

    /**
     * Flatten function expression binary operator
     *
     * PLUS
     *
     * 1. Flatten nested fexprs
     * (+ f1 (+ f2 f3))  =>  (+ f1 f2 f3)
     *
     * 2. Remove identity
     * (+ f 0)           =>  f
     * (+ f f)           =>  (* 2 f)
     *
     * 3. Handle constants
     * (+ 3 4)           =>  7
     *
     * MUL
     *
     * 1. Flatten nested fexprs
     * (* f1 (* f2 f3))  =>  (* f1 f2 f3)
     *
     * 2. Remove identity
     * (* f 1)           =>  f
     * (* f 0)           =>  0
     * (* f (- 1))       =>  (- f)
     *
     * 3. Handle constants
     * (* 3 4)           =>  12
     *
     * DIV
     *
     * 1. Remove identity
     * (/ f 1)           =>  f
     * (/ 0 f)           =>  0
     * (/ f f)           =>  1
     *
     * 2. Handle division by zero error
     * (/ f 0)           =>  throw
     *
     * 3. Handle constants
     * (/ 1 4)           =>  0.25
     *
     * MINUS
     *
     * 1. Remove identity
     * (- f 0)           =>  f
     * (- f f)           =>  0
     *
     * 2. Handle constants
     * (- 5 3)           =>  2
     */
    // loki::FunctionExpression translate_impl(const loki::FunctionExpressionBinaryOperatorImpl& fexpr);

    /**
     * Flatten function expression multi operator
     *
     * PLUS
     *
     * 1. Flatten nested fexprs
     * (+ f1 (+ f2 f3))  =>  (+ f1 f2 f3)
     *
     * 2. Remove identity
     * (+ f1 f2 0)       =>  (+ f1 f2)
     * (+ f)             =>  f
     * (+ )              =>  0
     *
     * 3. Combine constants
     * (+ f 3 4)         =>  (+ f 7)
     *
     * 4. Handle repeated terms
     * (+ f1 f1 f2)      =>  (+ (* 2 f1) f2)
     *
     * MUL
     *
     * 1. Flatten nested fexprs
     * (* f1 (* f2 f3))  =>  (* f1 f2 f3)
     *
     * 2. Remove identity
     * (* f (- 1))       =>  (- f)
     * (* f1 f2 0)       =>  0
     * (* f1 f2 1)       =>  (* f1 f2)
     * (* f)             =>  f
     * (* )              =>  1
     *
     * 3. Combine constants
     * (* f 3 4)         =>  (* f 12)
     */
    // loki::FunctionExpression translate_impl(const loki::FunctionExpressionMultiOperatorImpl& fexpr);

    loki::Problem run_impl(const loki::ProblemImpl& problem);

public:
    explicit ToENFTranslator(loki::PDDLRepositories& pddl_repositories);
};
}

#endif