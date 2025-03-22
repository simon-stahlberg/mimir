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

using namespace mimir::formalism;

namespace mimir::languages::dl
{

/* Concepts */

void RecurseVisitor::visit(ConceptBot constructor) {}
void RecurseVisitor::visit(ConceptTop constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<StaticTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<FluentTag> constructor) {}
void RecurseVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor) {}
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
void RecurseVisitor::visit(RoleAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<StaticTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<FluentTag> constructor) {}
void RecurseVisitor::visit(RoleAtomicGoal<DerivedTag> constructor) {}
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
void RecurseVisitor::visit(BooleanAtomicState<StaticTag> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<FluentTag> constructor) {}
void RecurseVisitor::visit(BooleanAtomicState<DerivedTag> constructor) {}
void RecurseVisitor::visit(BooleanNonempty<ConceptTag> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(BooleanNonempty<RoleTag> constructor) { constructor->get_constructor()->accept(*this); }

/* Numericals */
void RecurseVisitor::visit(NumericalCount<ConceptTag> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(NumericalCount<RoleTag> constructor) { constructor->get_constructor()->accept(*this); }
void RecurseVisitor::visit(NumericalDistance constructor)
{
    constructor->get_left_concept()->accept(*this);
    constructor->get_role()->accept(*this);
    constructor->get_left_concept()->accept(*this);
}

}
