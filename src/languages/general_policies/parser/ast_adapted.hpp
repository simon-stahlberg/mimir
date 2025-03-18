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

#ifndef SRC_LANGUAGES_GENERAL_POLICIES_PARSER_AST_ADAPTED_HPP_
#define SRC_LANGUAGES_GENERAL_POLICIES_PARSER_AST_ADAPTED_HPP_

#include "ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

// We need to tell fusion about our rexpr and rexpr_key_value
// to make them a first-class fusion citizens

BOOST_FUSION_ADAPT_TPL_STRUCT((D),
                              (mimir::languages::general_policies::ast::NamedFeature)(D),
                              (std::string, name)(mimir::languages::dl::ast::Constructor<D>, feature))

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::PositiveBooleanCondition, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::NegativeBooleanCondition, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::GreaterNumericalCondition, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::EqualNumericalCondition, feature_name)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::PositiveBooleanEffect, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::NegativeBooleanEffect, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::UnchangedBooleanEffect, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::IncreaseNumericalEffect, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::DecreaseNumericalEffect, feature_name)
BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::UnchangedNumericalEffect, feature_name)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::Rule, conditions, effects)

BOOST_FUSION_ADAPT_STRUCT(mimir::languages::general_policies::ast::GeneralPolicy, boolean_features, numerical_features, rules)

#endif
