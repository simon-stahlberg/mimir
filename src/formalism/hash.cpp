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

#include "mimir/formalism/hash.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{
size_t UniquePDDLHasher<const ActionImpl*>::operator()(const ActionImpl* e) const
{
    return UniquePDDLHashCombiner()(e->get_name(),
                                    e->get_parameters(),
                                    e->get_conditions<Static>(),
                                    e->get_conditions<Fluent>(),
                                    e->get_conditions<Derived>(),
                                    e->get_simple_effects(),
                                    e->get_conditional_effects(),
                                    e->get_universal_effects(),
                                    e->get_function_expression());
}

template<PredicateCategory P>
size_t UniquePDDLHasher<const AtomImpl<P>*>::operator()(const AtomImpl<P>* e) const
{
    return UniquePDDLHashCombiner()(e->get_predicate(), e->get_terms());
}

template size_t UniquePDDLHasher<const AtomImpl<Static>*>::operator()(const AtomImpl<Static>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Fluent>*>::operator()(const AtomImpl<Fluent>* e) const;
template size_t UniquePDDLHasher<const AtomImpl<Derived>*>::operator()(const AtomImpl<Derived>* e) const;

}
