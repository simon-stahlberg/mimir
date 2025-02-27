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

void ConstructorVisitor<Concept>::initialize(ConstructorVisitor<Role>& role_visitor) { m_role_visitor = &role_visitor; }

void ConstructorVisitor<Concept>::visit(ConceptBot constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptTop constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor) {}
void ConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    constructor->get_concept_left()->accept(*this);
    constructor->get_concept_right()->accept(*this);
}
void ConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    constructor->get_concept_left()->accept(*this);
    constructor->get_concept_right()->accept(*this);
}
void ConstructorVisitor<Concept>::visit(ConceptNegation constructor) { constructor->get_concept()->accept(*this); }
void ConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    constructor->get_role()->accept(*m_role_visitor);
    constructor->get_concept()->accept(*this);
}
void ConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role()->accept(*m_role_visitor);
    constructor->get_concept()->accept(*this);
}
void ConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_role_left()->accept(*m_role_visitor);
    constructor->get_role_right()->accept(*m_role_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_role_left()->accept(*m_role_visitor);
    constructor->get_role_right()->accept(*m_role_visitor);
}
void ConstructorVisitor<Concept>::visit(ConceptNominal constructor) {}

/* Roles */
void ConstructorVisitor<Role>::initialize(ConstructorVisitor<Concept>& concept_visitor) { m_concept_visitor = &concept_visitor; }

void ConstructorVisitor<Role>::visit(RoleUniversal constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor) {}
void ConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor) {}
void ConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    constructor->get_role_left()->accept(*this);
    constructor->get_role_right()->accept(*this);
}
void ConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    constructor->get_role_left()->accept(*this);
    constructor->get_role_right()->accept(*this);
}
void ConstructorVisitor<Role>::visit(RoleComplement constructor) { constructor->get_role()->accept(*this); }
void ConstructorVisitor<Role>::visit(RoleInverse constructor) { constructor->get_role()->accept(*this); }
void ConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    constructor->get_role_left()->accept(*this);
    constructor->get_role_right()->accept(*this);
}
void ConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void ConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role()->accept(*this); }
void ConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    constructor->get_role()->accept(*this);
    constructor->get_concept()->accept(*m_concept_visitor);
}
void ConstructorVisitor<Role>::visit(RoleIdentity constructor) { constructor->get_concept()->accept(*m_concept_visitor); }

}
