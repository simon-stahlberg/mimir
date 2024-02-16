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

#include <mimir/formalism/domain/parsers/term.hpp>

#include <mimir/formalism/domain/parsers/object.hpp>
#include <mimir/formalism/domain/parsers/variable.hpp>


namespace mimir 
{
    TermVisitor::TermVisitor(PDDLFactories& factories_) : factories(factories_) { }

    Term TermVisitor::operator()(const loki::pddl::TermObjectImpl& node) {
        std::cout << "Term parse" << std::endl;
        return factories.terms.get_or_create<TermObjectImpl>(parse(node.get_object(), factories));
    }

    Term TermVisitor::operator()(const loki::pddl::TermVariableImpl& node) {
        std::cout << "Term parse" << std::endl;
        return factories.terms.get_or_create<TermVariableImpl>(parse(node.get_variable(), factories));
    }

    Term parse(loki::pddl::Term term, PDDLFactories& factories) {
        std::cout << "Term parse" << std::endl;
        return std::visit(TermVisitor(factories), *term);
    }

    TermList parse(loki::pddl::TermList term_list, PDDLFactories& factories) {
        std::cout << "Terms parse" << std::endl;
        auto result_term_list = TermList();
        for (const auto& term : term_list) {
            result_term_list.push_back(parse(term, factories));
        }
        return result_term_list;
    }
}
