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

#include <mimir/formalism/factories.hpp>
#include <mimir/formalism/parsers/object.hpp>
#include <mimir/formalism/parsers/term.hpp>
#include <mimir/formalism/parsers/variable.hpp>

namespace mimir
{
TermVisitor::TermVisitor(PDDLFactories& factories_) : factories(factories_) {}

Term TermVisitor::operator()(const loki::pddl::TermObjectImpl& node)
{
    return factories.terms.get_or_create<TermObjectImpl>(parse(node.get_object(), factories));
}

Term TermVisitor::operator()(const loki::pddl::TermVariableImpl& node)
{
    return factories.terms.get_or_create<TermVariableImpl>(parse(node.get_variable(), factories));
}

Term parse(loki::pddl::Term term, PDDLFactories& factories) { return std::visit(TermVisitor(factories), *term); }

TermList parse(loki::pddl::TermList term_list, PDDLFactories& factories)
{
    auto result_term_list = TermList();
    for (const auto& term : term_list)
    {
        result_term_list.push_back(parse(term, factories));
    }
    return result_term_list;
}
}
