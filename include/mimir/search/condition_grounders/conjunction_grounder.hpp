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

#ifndef MIMIR_SEARCH_LIFTED_CONJUNCTION_GROUNDERS_HPP_
#define MIMIR_SEARCH_LIFTED_CONJUNCTION_GROUNDERS_HPP_

#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/declarations.hpp"

#include <memory>

namespace mimir
{

class LiftedConjunctionGrounder
{
private:
    ConditionGrounder m_condition_grounder;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

public:
    LiftedConjunctionGrounder(Problem problem,
                              VariableList variables,
                              LiteralList<Static> static_literals,
                              LiteralList<Fluent> fluent_literals,
                              LiteralList<Derived> derived_literals,
                              std::shared_ptr<PDDLRepositories> pddl_repositories);

    std::pair<std::vector<ObjectList>, std::tuple<std::vector<GroundLiteralList<Static>>, std::vector<GroundLiteralList<Fluent>>, std::vector<GroundLiteralList<Derived>>>> ground(State state);
};

}

#endif
