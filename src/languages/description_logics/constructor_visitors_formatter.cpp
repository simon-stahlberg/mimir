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

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_keywords.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{

FormatterVisitor::FormatterVisitor(std::ostream& out) : m_out(out) {}

/* Concepts */
void FormatterVisitor::visit(ConceptBot constructor) { m_out << keywords::concept_bot; }
void FormatterVisitor::visit(ConceptTop constructor) { m_out << keywords::concept_top; }
void FormatterVisitor::visit(ConceptAtomicState<StaticTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<FluentTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicState<DerivedTag> constructor)
{
    m_out << keywords::concept_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(ConceptAtomicGoal<StaticTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptAtomicGoal<FluentTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptAtomicGoal<DerivedTag> constructor)
{
    m_out << keywords::concept_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(ConceptIntersection constructor)
{
    m_out << keywords::concept_intersection << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptUnion constructor)
{
    m_out << keywords::concept_union << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptNegation constructor)
{
    m_out << keywords::concept_negation << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptValueRestriction constructor)
{
    m_out << keywords::concept_value_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptExistentialQuantification constructor)
{
    m_out << keywords::concept_existential_quantification << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(ConceptRoleValueMapContainment constructor)
{
    m_out << keywords::concept_role_value_map_containment << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(ConceptRoleValueMapEquality constructor)
{
    m_out << keywords::concept_role_value_map_equality << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(ConceptNominal constructor) { m_out << keywords::concept_nominal << " " << constructor->get_object()->get_name(); }

/* Roles */

void FormatterVisitor::visit(RoleUniversal constructor) { m_out << keywords::role_universal; }
void FormatterVisitor::visit(RoleAtomicState<StaticTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<FluentTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicState<DerivedTag> constructor)
{
    m_out << keywords::role_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(RoleAtomicGoal<StaticTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleAtomicGoal<FluentTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleAtomicGoal<DerivedTag> constructor)
{
    m_out << keywords::role_atomic_goal << " \"" << constructor->get_predicate()->get_name() << "\" " << (constructor->get_polarity() ? "true" : "false");
}
void FormatterVisitor::visit(RoleIntersection constructor)
{
    m_out << keywords::role_intersection << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleUnion constructor)
{
    m_out << keywords::role_union << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleComplement constructor)
{
    m_out << keywords::role_complement << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleInverse constructor)
{
    m_out << keywords::role_inverse << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleComposition constructor)
{
    m_out << keywords::role_composition << " ";
    constructor->get_left_role()->accept(*this);
    m_out << " ";
    constructor->get_right_role()->accept(*this);
}
void FormatterVisitor::visit(RoleTransitiveClosure constructor)
{
    m_out << keywords::role_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleReflexiveTransitiveClosure constructor)
{
    m_out << keywords::role_reflexive_transitive_closure << " ";
    constructor->get_role()->accept(*this);
}
void FormatterVisitor::visit(RoleRestriction constructor)
{
    m_out << keywords::role_restriction << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_concept()->accept(*this);
}
void FormatterVisitor::visit(RoleIdentity constructor)
{
    m_out << keywords::role_identity << " ";
    constructor->get_concept()->accept(*this);
}

/**
 * Booleans
 */

void FormatterVisitor::visit(BooleanAtomicState<StaticTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(BooleanAtomicState<FluentTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(BooleanAtomicState<DerivedTag> constructor)
{
    m_out << keywords::boolean_atomic_state << " \"" << constructor->get_predicate()->get_name() << "\"";
}
void FormatterVisitor::visit(BooleanNonempty<ConceptTag> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    constructor->get_constructor()->accept(*this);
}
void FormatterVisitor::visit(BooleanNonempty<RoleTag> constructor)
{
    m_out << keywords::boolean_nonempty << " ";
    constructor->get_constructor()->accept(*this);
}

/**
 * Numericals
 */

void FormatterVisitor::visit(NumericalCount<ConceptTag> constructor)
{
    m_out << keywords::numerical_count << " ";
    constructor->get_constructor()->accept(*this);
}
void FormatterVisitor::visit(NumericalCount<RoleTag> constructor)
{
    m_out << keywords::numerical_count << " ";
    constructor->get_constructor()->accept(*this);
}
void FormatterVisitor::visit(NumericalDistance constructor)
{
    m_out << keywords::numerical_distance << " ";
    constructor->get_left_concept()->accept(*this);
    m_out << " ";
    constructor->get_role()->accept(*this);
    m_out << " ";
    constructor->get_right_concept()->accept(*this);
}

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, const Constructor<D>& constructor)
{
    auto visitor = FormatterVisitor(out);

    constructor->accept(visitor);

    return out;
}

template std::ostream& operator<<(std::ostream& out, const Constructor<ConceptTag>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<RoleTag>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<BooleanTag>& data);
template std::ostream& operator<<(std::ostream& out, const Constructor<NumericalTag>& data);

}