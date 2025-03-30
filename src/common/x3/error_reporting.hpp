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

#ifndef SRC_COMMON_X3_ERROR_REPORTING_HPP_
#define SRC_COMMON_X3_ERROR_REPORTING_HPP_

#include "config.hpp"

#include <sstream>

namespace mimir::x3
{
/* Combined error handler and output stream. */
class X3ErrorHandler
{
private:
    // Order of initialization matters
    std::ostringstream m_error_stream;
    error_handler_type m_error_handler;

public:
    X3ErrorHandler(iterator_type first, iterator_type last, const std::string& file) : m_error_handler(error_handler_type(first, last, m_error_stream, file)) {}

    // delete copy and move to avoid dangling references.
    X3ErrorHandler(const X3ErrorHandler& other) = delete;
    X3ErrorHandler& operator=(const X3ErrorHandler& other) = delete;
    X3ErrorHandler(X3ErrorHandler&& other) = delete;
    X3ErrorHandler& operator=(X3ErrorHandler&& other) = delete;

    const error_handler_type& get_error_handler() const { return m_error_handler; }

    error_handler_type& get_error_handler() { return m_error_handler; }

    const std::ostringstream& get_error_stream() const { return m_error_stream; }

    std::ostringstream& get_error_stream() { return m_error_stream; }
};
}

#endif
