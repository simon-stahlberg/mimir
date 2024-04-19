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

#ifndef MIMIR_FORMALISM_TRANSLATIONS_HPP_
#define MIMIR_FORMALISM_TRANSLATIONS_HPP_

#include "mimir/formalism/translators/move_existential_quantifiers.hpp"
#include "mimir/formalism/translators/remove_types.hpp"
#include "mimir/formalism/translators/remove_universal_quantifiers.hpp"
#include "mimir/formalism/translators/rename_quantified_variables.hpp"
#include "mimir/formalism/translators/simplify_goal.hpp"
#include "mimir/formalism/translators/split_disjunctive_conditions.hpp"
#include "mimir/formalism/translators/to_disjunctive_normal_form.hpp"
#include "mimir/formalism/translators/to_effect_normal_form.hpp"
#include "mimir/formalism/translators/to_mimir_structures.hpp"
#include "mimir/formalism/translators/to_negation_normal_form.hpp"

#endif