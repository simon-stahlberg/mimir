/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_PARSER_AST_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_PARSER_AST_HPP_

#include "../../description_logics/parser/ast.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <map>
#include <sstream>
#include <vector>

namespace mimir::languages::general_policies::ast
{
///////////////////////////////////////////////////////////////////////////
//  The AST
///////////////////////////////////////////////////////////////////////////
namespace x3 = boost::spirit::x3;

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
struct NamedFeature;

struct Condition;

struct PositiveBooleanCondition;
struct NegativeBooleanCondition;
struct GreaterNumericalCondition;
struct EqualNumericalCondition;

struct Effect;

struct PositiveBooleanEffect;
struct NegativeBooleanEffect;
struct UnchangedBooleanEffect;
struct IncreaseNumericalEffect;
struct DecreaseNumericalEffect;
struct UnchangedNumericalEffect;

struct Rule;
struct GeneralPolicy;

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
struct NamedFeature
{
    std::string name;
    dl::ast::Constructor<D> feature;
};

struct Condition :
    x3::position_tagged,
    x3::variant<x3::forward_ast<PositiveBooleanCondition>,
                x3::forward_ast<NegativeBooleanCondition>,
                x3::forward_ast<GreaterNumericalCondition>,
                x3::forward_ast<EqualNumericalCondition>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct PositiveBooleanCondition : x3::position_tagged
{
    std::string feature_name;
};
struct NegativeBooleanCondition : x3::position_tagged
{
    std::string feature_name;
};
struct GreaterNumericalCondition : x3::position_tagged
{
    std::string feature_name;
};
struct EqualNumericalCondition : x3::position_tagged
{
    std::string feature_name;
};

struct Effect :
    x3::position_tagged,
    x3::variant<x3::forward_ast<PositiveBooleanEffect>,
                x3::forward_ast<NegativeBooleanEffect>,
                x3::forward_ast<UnchangedBooleanEffect>,
                x3::forward_ast<IncreaseNumericalEffect>,
                x3::forward_ast<DecreaseNumericalEffect>,
                x3::forward_ast<UnchangedNumericalEffect>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct PositiveBooleanEffect : x3::position_tagged
{
    std::string feature_name;
};
struct NegativeBooleanEffect : x3::position_tagged
{
    std::string feature_name;
};
struct UnchangedBooleanEffect : x3::position_tagged
{
    std::string feature_name;
};
struct IncreaseNumericalEffect : x3::position_tagged
{
    std::string feature_name;
};
struct DecreaseNumericalEffect : x3::position_tagged
{
    std::string feature_name;
};
struct UnchangedNumericalEffect : x3::position_tagged
{
    std::string feature_name;
};

struct Rule : x3::position_tagged
{
    std::vector<Condition> conditions;
    std::vector<Effect> effects;
};

struct GeneralPolicy : x3::position_tagged
{
    std::vector<NamedFeature<dl::BooleanTag>> boolean_features;
    std::vector<NamedFeature<dl::NumericalTag>> numerical_features;
    std::vector<Rule> rules;
};

}

#endif
