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

#ifndef MIMIR_FORMALISM_NUMERIC_FLUENT_HPP_
#define MIMIR_FORMALISM_NUMERIC_FLUENT_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class NumericFluentImpl
{
private:
    Index m_index;
    GroundFunction m_function;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    NumericFluentImpl(Index index, GroundFunction function, double number);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NumericFluentImpl(const NumericFluentImpl& other) = delete;
    NumericFluentImpl& operator=(const NumericFluentImpl& other) = delete;
    NumericFluentImpl(NumericFluentImpl&& other) = default;
    NumericFluentImpl& operator=(NumericFluentImpl&& other) = default;

    Index get_index() const;
    const GroundFunction& get_function() const;
    double get_number() const;
};

extern std::ostream& operator<<(std::ostream& out, const NumericFluentImpl& element);

extern std::ostream& operator<<(std::ostream& out, NumericFluent element);

}

#endif
