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

#include "mimir/languages/description_logics/formatter.hpp"

#include "mimir/languages/description_logics/cnf_grammar.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructors.hpp"
#include "mimir/languages/description_logics/cnf_grammar_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/grammar_visitor_formatter.hpp"

namespace mimir
{
namespace languages::dl
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const IConstructor<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<NumericalTag>& element);

namespace cnf_grammar
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const IConstructor<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<NumericalTag>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<NumericalTag>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<NumericalTag>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<NumericalTag>& element);

std::ostream& operator<<(std::ostream& out, const Grammar& element) { return mimir::print(out, element); }
}  // end cnf_grammar

namespace grammar
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const IConstructor<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const IConstructor<NumericalTag>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<NumericalTag>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<NumericalTag>& element);

std::ostream& operator<<(std::ostream& out, const Grammar& element) { return mimir::print(out, element); }

}  // end grammar

}  // end languages::dl

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<D>& element)
{
    auto visitor = mimir::languages::dl::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<D>& element)
{
    auto visitor = mimir::languages::dl::cnf_grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<D>& element)
{
    auto visitor = mimir::languages::dl::cnf_grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<D>& element)
{
    auto visitor = mimir::languages::dl::cnf_grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<D>& element)
{
    auto visitor = mimir::languages::dl::cnf_grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<mimir::languages::dl::NumericalTag>& element);

std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::Grammar& element)
{
    auto visitor = mimir::languages::dl::cnf_grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<D>& element)
{
    auto visitor = mimir::languages::dl::grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<D>& element)
{
    auto visitor = mimir::languages::dl::grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<mimir::languages::dl::NumericalTag>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<D>& element)
{
    auto visitor = mimir::languages::dl::grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<mimir::languages::dl::NumericalTag>& element);

std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::Grammar& element)
{
    auto visitor = mimir::languages::dl::grammar::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}
}
