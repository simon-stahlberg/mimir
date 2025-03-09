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

FormatterConstructorVisitor<Concept>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

/* Concepts */
void FormatterConstructorVisitor<Concept>::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterConstructorVisitor<Concept>::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Static> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Fluent> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicState<Derived> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Static> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Fluent> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptAtomicGoal<Derived> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Concept>::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_left_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_right_role()->accept(*m_role_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_left_role()->accept(*m_role_visitor);
    m_out << " ";
    constructor->get_right_role()->accept(*m_role_visitor);
}
void FormatterConstructorVisitor<Concept>::visit(ConceptNominal constructor)
{
    m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name();
}

/* Roles */
FormatterConstructorVisitor<Role>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Role>::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Static> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Fluent> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicState<Derived> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Static> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Fluent> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role>::visit(RoleAtomicGoal<Derived> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->is_negated() ? "false" : "true");
}
void FormatterConstructorVisitor<Role>::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterConstructorVisitor<Role>::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*m_concept_visitor);
}
void FormatterConstructorVisitor<Role>::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(*m_concept_visitor);
}

template<FeatureCategory D>
extern std::ostream& operator<<(std::ostream& out, const Constructor<D>& constructor)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(out);
    auto role_visitor = FormatterConstructorVisitor<Role>(out);
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

/**
 * Booleans
 */

FormatterConstructorVisitor<Boolean>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Static> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Fluent> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Boolean>::visit(BooleanAtomicState<Derived> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterConstructorVisitor<Boolean>::visit(BooleanNonempty<Concept> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    auto concept_visitor = FormatterConstructorVisitor<Concept>(m_out);
    constructor->get_constructor()->accept(concept_visitor);
}
void FormatterConstructorVisitor<Boolean>::visit(BooleanNonempty<Role> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    auto role_visitor = FormatterConstructorVisitor<Role>(m_out);
    constructor->get_constructor()->accept(role_visitor);
}

/**
 * Numericals
 */

FormatterConstructorVisitor<Numerical>::FormatterConstructorVisitor(std::ostream& out) : m_out(out) {}

void FormatterConstructorVisitor<Numerical>::visit(NumericalCount<Concept> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    auto concept_visitor = FormatterConstructorVisitor<Concept>(m_out);
    constructor->get_constructor()->accept(concept_visitor);
}
void FormatterConstructorVisitor<Numerical>::visit(NumericalCount<Role> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    auto role_visitor = FormatterConstructorVisitor<Role>(m_out);
    constructor->get_constructor()->accept(role_visitor);
}
void FormatterConstructorVisitor<Numerical>::visit(NumericalDistance constructor)
{
    auto concept_visitor = FormatterConstructorVisitor<Concept>(m_out);
    auto role_visitor = FormatterConstructorVisitor<Role>(m_out);
    m_out << keywords::numerical_distance << " ";
    constructor->get_left_concept()->accept(concept_visitor);
    m_out << " ";
    constructor->get_role()->accept(role_visitor);
    m_out << " ";
    constructor->get_right_concept()->accept(concept_visitor);
}

template std::ostream& operator<<(std::ostream& out, const Constructor<Concept>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<Role>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<Boolean>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<Numerical>& data);

}