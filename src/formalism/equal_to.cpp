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

#include "mimir/formalism/equal_to.hpp"

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
bool UniquePDDLEqualTo<const ActionImpl*>::operator()(const ActionImpl* l, const ActionImpl* r) const
{
    if (&l != &r)
    {
        return (l->get_name() == r->get_name()) && (l->get_parameters() == r->get_parameters()) && (l->get_conditions<Static>() == r->get_conditions<Static>())
               && (l->get_conditions<Fluent>() == r->get_conditions<Fluent>()) && (l->get_conditions<Derived>() == r->get_conditions<Derived>())
               && (l->get_simple_effects() == r->get_simple_effects()) && (l->get_conditional_effects() == r->get_conditional_effects())
               && (l->get_universal_effects() == r->get_universal_effects()) && (l->get_function_expression() == r->get_function_expression());
    }
    return true;
}

template<PredicateCategory P>
bool UniquePDDLEqualTo<const AtomImpl<P>*>::operator()(const AtomImpl<P>* l, const AtomImpl<P>* r) const
{
    if (&l != &r)
    {
        return (l->get_predicate() == r->get_predicate()) && (l->get_terms() == r->get_terms());
    }
    return true;
}

template bool UniquePDDLEqualTo<const AtomImpl<Static>*>::operator()(const AtomImpl<Static>* l, const AtomImpl<Static>* r) const;
template bool UniquePDDLEqualTo<const AtomImpl<Fluent>*>::operator()(const AtomImpl<Fluent>* l, const AtomImpl<Fluent>* r) const;
template bool UniquePDDLEqualTo<const AtomImpl<Derived>*>::operator()(const AtomImpl<Derived>* l, const AtomImpl<Derived>* r) const;

}
