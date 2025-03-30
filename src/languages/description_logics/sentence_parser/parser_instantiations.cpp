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

namespace mimir::languages::dl::sentence_parser
{
using iterator_type = mimir::x3::iterator_type;
using phrase_context_type = mimir::x3::phrase_context_type;
using context_type = mimir::x3::context_type;

BOOST_SPIRIT_INSTANTIATE(concept_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_bot_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_top_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_atomic_state_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_atomic_goal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_intersection_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_union_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_negation_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_value_restriction_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_existential_quantification_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_role_value_map_containment_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_role_value_map_equality_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_nominal_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(role_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_universal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_atomic_state_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_atomic_goal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_intersection_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_union_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_complement_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_inverse_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_composition_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_transitive_closure_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_reflexive_transitive_closure_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_restriction_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_identity_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(concept_or_role_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(boolean_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(boolean_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(boolean_atomic_state_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(boolean_nonempty_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(numerical_root_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(numerical_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(numerical_count_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(numerical_distance_type, iterator_type, context_type)

}
