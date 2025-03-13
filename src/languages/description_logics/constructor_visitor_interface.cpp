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

void RecurseVisitor::visit(ConceptBot constructor) {}
void RecurseVisitor::visit(ConceptTop constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Static> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Static> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Fluent> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<Derived> constructor) {}
void RecurseVisitor::visit(ConceptIntersection constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptUnion constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_right_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptNegation constructor) { constructor->get_concept()->accept(*this); }
void RecurseVisitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(ConceptNominal constructor) {}

/* Roles */
void RecurseVisitor::visit(RoleUniversal constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Static> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Static> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Fluent> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<Derived> constructor) {}
void RecurseVisitor::visit(RoleIntersection constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleUnion constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleComplement constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleInverse constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleComposition constructor)
{
    constructor->get_left_role()->accept(*this);
    constructor->get_right_role()->accept(*this);
}
void RecurseVisitor::visit(RoleTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void RecurseVisitor::visit(RoleRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*this);
}
void RecurseVisitor::visit(RoleIdentity constructor) { constructor->get_concept()->accept(*this); }

/* Booleans */
void RecurseVisitor::visit(BooleanAtomicState<Static> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Fluent> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<Derived> constructor) {}
void RecurseVisitor::visit(BooleanNonempty<Concept> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(BooleanNonempty<Role> constructor) { constructor->get_constructor()->accept(*this); }

/* Numericals */
void RecurseVisitor::visit(NumericalCount<Concept> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(NumericalCount<Role> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_role()->accept(*this);
    constructor->get_left_concept()->accept(*this);
}

}
