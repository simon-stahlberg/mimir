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

#include "mimir/languages/description_logics/constructor_visitors_formatter.hpp"

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/constructors.hpp"

namespace mimir::dl
{

BNFFormatterVisitor::BNFFormatterVisitor(std::ostream& out) : m_out(out) {}

/* Concepts */
void BNFFormatterVisitor::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void BNFFormatterVisitor::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void BNFFormatterVisitor::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_concept_left()->accept(*this);
    m_out << " ";
    constructor->get_concept_right()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_concept_left()->accept(*this);
    m_out << " ";
    constructor->get_concept_right()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_role_left()->accept(*this);
    m_out << " ";
    constructor->get_role_right()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_role_left()->accept(*this);
    m_out << " ";
    constructor->get_role_right()->accept(*this);
}
void BNFFormatterVisitor::visit(ConceptNominal constructor) { m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name(); }

/* Roles */
void BNFFormatterVisitor::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void BNFFormatterVisitor::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void BNFFormatterVisitor::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void BNFFormatterVisitor::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_role_left()->accept(*this);
    m_out << " ";
    constructor->get_role_right()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_role_left()->accept(*this);
    m_out << " ";
    constructor->get_role_right()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_role_left()->accept(*this);
    m_out << " ";
    constructor->get_role_right()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void BNFFormatterVisitor::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(*this);
}

template<ConstructorTag D>
extern std::ostream& operator<<(std::ostream& out, const std::tuple<Constructor<D>, BNFFormatterVisitorTag>& data)
{
    const auto& [constructor, tag] = data;
    auto visitor = BNFFormatterVisitor(out);
    constructor->accept(visitor);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const std::tuple<Constructor<Concept>, BNFFormatterVisitorTag>& data);
template std::ostream& operator<<(std::ostream& out, const std::tuple<Constructor<Role>, BNFFormatterVisitorTag>& data);

template<ConstructorTag D>
std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, Constructor<D>, BNFFormatterVisitorTag>& data)
{
    const auto& [name, constructor, tag] = data;
    out << name << " ::= " << std::make_tuple(constructor, tag);
    return out;
}

template std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, Constructor<Concept>, BNFFormatterVisitorTag>& data);
template std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, Constructor<Role>, BNFFormatterVisitorTag>& data);

}