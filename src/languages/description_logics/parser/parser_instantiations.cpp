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

#include "parser_def.hpp"

#include <mimir/languages/description_logics/parser/config.hpp>

namespace mimir::dl::parser
{
using iterator_type = mimir::dl::iterator_type;
using phrase_context_type = mimir::dl::phrase_context_type;
using context_type = mimir::dl::context_type;

BOOST_SPIRIT_INSTANTIATE(concept_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_predicate_state_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_predicate_goal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_and_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_non_terminal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(concept_derivation_rule_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(role_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_predicate_state_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_predicate_goal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_and_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_non_terminal_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_choice_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(role_derivation_rule_type, iterator_type, context_type)

BOOST_SPIRIT_INSTANTIATE(derivation_rule_type, iterator_type, context_type)
BOOST_SPIRIT_INSTANTIATE(grammar_type, iterator_type, context_type)

}
