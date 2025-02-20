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

#include "mimir/languages/description_logics/grammar_constructor_visitor_interface.hpp"

#include "grammar_constructor_visitor_interface_impl.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/grammar_containers.hpp"

namespace mimir::dl::grammar
{

/* Concepts */
void Visitor::visit(DerivationRule<Concept> constructor)
{
    constructor->get_non_terminal()->accept(*this);
    this->visit(constructor->get_constructor_or_non_terminals());
}
void Visitor::visit(NonTerminal<Concept> constructor) {}
void Visitor::visit(ConstructorOrNonTerminal<Concept> constructor) { this->visit(constructor->get_constructor_or_non_terminal()); }
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
    constructor->get_concept_or_non_terminal_left()->accept(*this);
    constructor->get_concept_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(ConceptUnion constructor)
{
    constructor->get_concept_or_non_terminal_left()->accept(*this);
    constructor->get_concept_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(ConceptNegation constructor) { constructor->get_concept_or_non_terminal()->accept(*this); }
void Visitor::visit(ConceptValueRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void Visitor::visit(ConceptExistentialQuantification constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void Visitor::visit(ConceptRoleValueMapContainment constructor)
{
    constructor->get_role_or_non_terminal_left()->accept(*this);
    constructor->get_role_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(ConceptRoleValueMapEquality constructor)
{
    constructor->get_role_or_non_terminal_left()->accept(*this);
    constructor->get_role_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(ConceptNominal constructor) {}

/* Roles */
void Visitor::visit(DerivationRule<Role> constructor)
{
    constructor->get_non_terminal()->accept(*this);
    this->visit(constructor->get_constructor_or_non_terminals());
}
void Visitor::visit(NonTerminal<Role> constructor) {}
void Visitor::visit(ConstructorOrNonTerminal<Role> constructor) { this->visit(constructor->get_constructor_or_non_terminal()); }
void Visitor::visit(RoleUniversal constructor) {}
void Visitor::visit(RoleAtomicState<Static> constructor) {}
void Visitor::visit(RoleAtomicState<Fluent> constructor) {}
void Visitor::visit(RoleAtomicState<Derived> constructor) {}
void Visitor::visit(RoleAtomicGoal<Static> constructor) {}
void Visitor::visit(RoleAtomicGoal<Fluent> constructor) {}
void Visitor::visit(RoleAtomicGoal<Derived> constructor) {}
void Visitor::visit(RoleIntersection constructor)
{
    constructor->get_role_or_non_terminal_left()->accept(*this);
    constructor->get_role_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(RoleUnion constructor)
{
    constructor->get_role_or_non_terminal_left()->accept(*this);
    constructor->get_role_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(RoleComplement constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void Visitor::visit(RoleInverse constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void Visitor::visit(RoleComposition constructor)
{
    constructor->get_role_or_non_terminal_left()->accept(*this);
    constructor->get_role_or_non_terminal_right()->accept(*this);
}
void Visitor::visit(RoleTransitiveClosure constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void Visitor::visit(RoleReflexiveTransitiveClosure constructor) { constructor->get_role_or_non_terminal()->accept(*this); }
void Visitor::visit(RoleRestriction constructor)
{
    constructor->get_role_or_non_terminal()->accept(*this);
    constructor->get_concept_or_non_terminal()->accept(*this);
}
void Visitor::visit(RoleIdentity constructor) { constructor->get_concept_or_non_terminal()->accept(*this); }

}
