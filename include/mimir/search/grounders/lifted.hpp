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

#ifndef MIMIR_SEARCH_GROUNDERS_LIFTED_HPP_
#define MIMIR_SEARCH_GROUNDERS_LIFTED_HPP_

#include "mimir/formalism/translator/delete_relax.hpp"
#include "mimir/search/grounders/interface.hpp"

#include <memory>

namespace mimir::search
{

class LiftedGrounder : public IGrounder
{
private:
    /* Delete free info to instantiate the grounded generators. */
    formalism::DeleteRelaxTranslator m_delete_relax_transformer;
    formalism::Problem m_delete_free_problem;
    formalism::ToObjectMap<formalism::Object> m_delete_free_object_to_unrelaxed_object;

public:
    explicit LiftedGrounder(formalism::Problem problem);
    LiftedGrounder(const LiftedGrounder& other) = delete;
    LiftedGrounder& operator=(const LiftedGrounder& other) = delete;
    LiftedGrounder(LiftedGrounder&& other) = delete;
    LiftedGrounder& operator=(LiftedGrounder&& other) = delete;

    formalism::GroundActionList create_ground_actions() const override;

    formalism::GroundAxiomList create_ground_axioms() const override;
};

}  // namespace mimir

#endif