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

#include "mimir/languages/description_logics/constructor_visitor_interface.hpp"

#include "mimir/languages/description_logics/constructors.hpp"

namespace mimir::dl
{

/* Concepts */

void Visitor::visit(ConceptBot constructor) {}
void Visitor::visit(ConceptTop constructor) {}
void Visitor::visit(ConceptAtomicState<Static> constructor) {}
void Visitor::visit(ConceptAtomicState<Fluent> constructor) {}
void Visitor::visit(ConceptAtomicState<Derived> constructor) {}
void Visitor::visit(ConceptAtomicGoal<Static> constructor) {}
void Visitor::visit(ConceptAtomicGoal<Fluent> constructor) {}
void Visitor::visit(ConceptAtomicGoal<Derived> constructor) {}
void Visitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void Visitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void Visitor::visit(ConceptNegation constructor) { constructor->get_concept()->accept(*this); }
void Visitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void Visitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void Visitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void Visitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void Visitor::visit(ConceptNominal constructor) {}

/* Roles */
void Visitor::visit(RoleUniversal constructor) {}
void Visitor::visit(RoleAtomicState<Static> constructor) {}
void Visitor::visit(RoleAtomicState<Fluent> constructor) {}
void Visitor::visit(RoleAtomicState<Derived> constructor) {}
void Visitor::visit(RoleAtomicGoal<Static> constructor) {}
void Visitor::visit(RoleAtomicGoal<Fluent> constructor) {}
void Visitor::visit(RoleAtomicGoal<Derived> constructor) {}
void Visitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void Visitor::visit(RoleUnion constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void Visitor::visit(RoleComplement constructor) { constructor->get_role()->accept(*this); }
void Visitor::visit(RoleInverse constructor) { constructor->get_role()->accept(*this); }
void Visitor::visit(RoleComposition constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void Visitor::visit(RoleTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void Visitor::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void Visitor::visit(RoleRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void Visitor::visit(RoleIdentity constructor) { constructor->get_concept()->accept(*this); }

/* Booleans */
void Visitor::visit(BooleanAtomicState<Static> constructor) {}
void Visitor::visit(BooleanAtomicState<Fluent> constructor) {}
void Visitor::visit(BooleanAtomicState<Derived> constructor) {}
void Visitor::visit(BooleanNonempty<Concept> constructor) { constructor->get_constructor()->accept(*this); }
void Visitor::visit(BooleanNonempty<Role> constructor) { constructor->get_constructor()->accept(*this); }

/* Numericals */
void Visitor::visit(NumericalCount<Concept> constructor) { constructor->get_constructor()->accept(*this); }
void Visitor::visit(NumericalCount<Role> constructor) { constructor->get_constructor()->accept(*this); }
void Visitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_role()->accept(*this);
    constructor->get_left_concept()->accept(*this);
}

}
