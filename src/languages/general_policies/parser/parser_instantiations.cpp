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

#include "../../../common/x3/config.hpp"
#include "parser_def.hpp"

namespace mimir::languages::general_policies::parser
{
using iterator_type = mimir::x3::iterator_type;
using phrase_context_type = mimir::x3::phrase_context_type;
using context_type = mimir::x3::context_type;

BOOST_SPIRIT_INSTANTIATE(named_concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(named_role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(named_boolean_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(named_numerical_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(positive_boolean_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_boolean_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(greater_numerical_condition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(equal_numerical_condition_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(positive_boolean_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(negative_boolean_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(unchanged_boolean_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(increase_numerical_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(decrease_numerical_effect_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(unchanged_numerical_effect_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(rule_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(general_policy_type, iterator_type, context_type)

}
