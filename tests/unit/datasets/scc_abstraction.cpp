/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#include "mimir/formalism/parser.hpp"

#include <gtest/gtest.h>
#include <mimir/datasets/scc_abstraction.hpp>

namespace mimir
{

TEST(MimirTests, DatasetsSccAbstractionTest)
{
    const auto domain_file = fs::path(std::string(DATA_DIR) + "ferry/domain.pddl");
    const auto problem_file = fs::path(std::string(DATA_DIR) + "ferry/test_problem.pddl");
    const auto parser = PDDLParser(domain_file, problem_file);
    const auto abstraction = SccAbstraction::create(parser.get_problem());
}

}
