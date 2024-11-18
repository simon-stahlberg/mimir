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

#ifndef MIMIR_FORMALISM_TERM_HPP_
#define MIMIR_FORMALISM_TERM_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir
{

class TermObjectImpl
{
private:
    Index m_index;
    Object m_object;

    // Below: add additional members if needed and initialize them in the constructor

    TermObjectImpl(Index index, Object object);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    TermObjectImpl(const TermObjectImpl& other) = delete;
    TermObjectImpl& operator=(const TermObjectImpl& other) = delete;
    TermObjectImpl(TermObjectImpl&& other) = default;
    TermObjectImpl& operator=(TermObjectImpl&& other) = default;

    std::string str() const;

    Index get_index() const;
    const Object& get_object() const;
};

class TermVariableImpl
{
private:
    Index m_index;
    Variable m_variable;

    TermVariableImpl(Index index, Variable variable);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    TermVariableImpl(const TermVariableImpl& other) = delete;
    TermVariableImpl& operator=(const TermVariableImpl& other) = delete;
    TermVariableImpl(TermVariableImpl&& other) = default;
    TermVariableImpl& operator=(TermVariableImpl&& other) = default;

    std::string str() const;

    Index get_index() const;
    const Variable& get_variable() const;
};

extern std::ostream& operator<<(std::ostream& out, const TermObjectImpl& element);
extern std::ostream& operator<<(std::ostream& out, const TermVariableImpl& element);

extern std::ostream& operator<<(std::ostream& out, Term element);

}

#endif
