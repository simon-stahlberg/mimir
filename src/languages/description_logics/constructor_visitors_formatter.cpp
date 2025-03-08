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

FormatterVisitor<Concept>::FormatterVisitor(std::ostream& out) : m_out(out) {}

/* Concepts */
void FormatterVisitor<Concept>::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterVisitor<Concept>::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Concept>::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor<Concept>::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor<Concept>::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_left_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_right_role()->accept(*m_role_visitor);
}
void FormatterVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_left_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_right_role()->accept(*m_role_visitor);
}
void FormatterVisitor<Concept>::visit(ConceptNominal constructor) { m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name(); }

/* Roles */
FormatterVisitor<Role>::FormatterVisitor(std::ostream& out) : m_out(out) {}

void FormatterVisitor<Role>::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterVisitor<Role>::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor<Role>::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*m_concept_visitor);
}
void FormatterVisitor<Role>::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(*m_concept_visitor);
}

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, const Constructor<D>& constructor)
{
    auto concept_visitor = FormatterVisitor<Concept>(out);
    auto role_visitor = FormatterVisitor<Role>(out);
    concept_visitor.initialize(role_visitor);
    role_visitor.initialize(concept_visitor);
    if constexpr (std::is_same_v<D, Concept>)
    {
        constructor->accept(concept_visitor);
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        constructor->accept(role_visitor);
    }
    return out;
}

template std::ostream& operator<<(std::ostream& out, const Constructor<Concept>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<Role>& data);

}