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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_RENAME_AXIOM_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_RENAME_AXIOM_VARIABLES_HPP_

#include "mimir/formalism/transformers/base_recurse.hpp"

#include <unordered_map>

namespace mimir
{

/**
 * TODO: rename axiom parameters to ?v_1,...,?v_n in order to remove duplicates that are only distinguishable by variable names.
 */
class RenameAxiomVariablesTramsformer : public BaseRecurseTransformer<RenameAxiomVariablesTramsformer>
{
private:
    /* Implement BaseRecurseTransformer interface. */
    friend class BaseRecurseTransformer<RenameAxiomVariablesTramsformer>;

    // Provide default implementations
    using BaseRecurseTransformer<RenameAxiomVariablesTramsformer>::prepare_impl;
    using BaseRecurseTransformer<RenameAxiomVariablesTramsformer>::transform_impl;

public:
    RenameAxiomVariablesTramsformer(PDDLRepositories& pddl_repositories);
};
}

#endif