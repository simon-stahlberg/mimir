/*
 * Copyright (C) 2023 Simon Stahlberg
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

#ifndef MIMIR_INCLUDE_ALGORITHMS_SOUFFLE_HPP_
#define MIMIR_INCLUDE_ALGORITHMS_SOUFFLE_HPP_

#include "parser/ParserDriver.h"
#include "souffle/io/IOSystem.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Source: https://github.com/souffle-lang/souffle/blob/master/tests/libsouffle_interface/vfs_overlay_test.cpp
 */

namespace mimir::datalog
{
extern int solve(const std::string& program, std::shared_ptr<souffle::WriteStreamFactory> write_stream_factory);
}

#endif
