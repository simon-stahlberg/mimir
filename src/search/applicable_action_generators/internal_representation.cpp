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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/internal_representation.hpp"

#include "mimir/common/translations.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/term.hpp"

#include <algorithm>

namespace mimir
{

/*
 * Struct Assignment and AssignmentPair
 */

Assignment::Assignment(size_t parameter_index, size_t object_id) : parameter_index(parameter_index), object_id(object_id) {}

AssignmentPair::AssignmentPair(size_t first_position, const Assignment& first_assignment, size_t second_position, const Assignment& second_assignment) :
    first_position(first_position),
    second_position(second_position),
    first_assignment(first_assignment),
    second_assignment(second_assignment)
{
}

}
