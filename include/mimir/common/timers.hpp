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

#ifndef MIMIR_FORMALISM_COMMON_TIMERS_HPP_
#define MIMIR_FORMALISM_COMMON_TIMERS_HPP_

#include <chrono>

class StopWatch
{
private:
    std::chrono::milliseconds m_timeout;
    std::chrono::steady_clock::time_point m_endTime;
    bool m_isRunning;

public:
    explicit StopWatch(size_t timeout_ms) : m_timeout(timeout_ms), m_isRunning(false) {}

    void start()
    {
        m_endTime = std::chrono::steady_clock::now() + m_timeout;
        m_isRunning = true;
    }

    bool has_finished() const
    {
        if (!m_isRunning)
        {
            return true;  // If never started, assume finished.
        }
        return std::chrono::steady_clock::now() >= m_endTime;
    }
};

#endif