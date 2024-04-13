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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "domain.hpp"

#include "action.hpp"
#include "axiom.hpp"
#include "function_skeleton.hpp"
#include "mimir/formalism/factories.hpp"
#include "object.hpp"
#include "predicate.hpp"
#include "requirements.hpp"
#include "type.hpp"

namespace mimir
{
Domain parse(loki::Domain domain, PDDLFactories& factories)
{
    return factories.get_or_create_domain(domain->get_name(),
                                          parse(domain->get_requirements(), factories),
                                          parse(domain->get_types(), factories),
                                          parse(domain->get_constants(), factories),
                                          parse(domain->get_predicates(), factories),
                                          parse(domain->get_derived_predicates(), factories),
                                          parse(domain->get_functions(), factories),
                                          parse(domain->get_actions(), factories),
                                          parse(domain->get_axioms(), factories));
}
}
