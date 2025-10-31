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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_FORMATTER_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_FORMATTER_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir
{
namespace languages::dl
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& constructor);

namespace cnf_grammar
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<D>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<D>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const SubstitutionRuleImpl<D>& element);

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);
}  // end cnf_grammar

namespace grammar
{
template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const IConstructor<D>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const NonTerminalImpl<D>& element);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const DerivationRuleImpl<D>& element);

extern std::ostream& operator<<(std::ostream& out, const Grammar& element);

}  // end grammar

}  // end languages::dl

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::IConstructor<D>& constructor);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::IConstructor<D>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::NonTerminalImpl<D>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::DerivationRuleImpl<D>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::SubstitutionRuleImpl<D>& element);

extern std::ostream& print(std::ostream& out, const mimir::languages::dl::cnf_grammar::Grammar& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::IConstructor<D>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::NonTerminalImpl<D>& element);

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::DerivationRuleImpl<D>& element);

extern std::ostream& print(std::ostream& out, const mimir::languages::dl::grammar::Grammar& element);
}

#endif
