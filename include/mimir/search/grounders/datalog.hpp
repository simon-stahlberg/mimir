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

#ifndef MIMIR_SEARCH_GROUNDERS_DATALOG_HPP_
#define MIMIR_SEARCH_GROUNDERS_DATALOG_HPP_

#include "mimir/search/grounders/interface.hpp"

#include <memory>

namespace mimir::search
{

class DatalogGrounder : public IGrounder
{
    formalism::GroundActionSet m_ground_actions;
    formalism::GroundAxiomSet m_ground_axioms;

public:
    explicit DatalogGrounder(formalism::Problem problem);
    DatalogGrounder(const DatalogGrounder& other) = delete;
    DatalogGrounder& operator=(const DatalogGrounder& other) = delete;
    DatalogGrounder(DatalogGrounder&& other) = delete;
    DatalogGrounder& operator=(DatalogGrounder&& other) = delete;

    formalism::GroundActionList create_ground_actions() const override;

    formalism::GroundAxiomList create_ground_axioms() const override;
};

}  // namespace mimir

#endif
