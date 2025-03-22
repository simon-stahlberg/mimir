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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_INTERFACE_HPP_

#include "mimir/languages/description_logics/constructor_interface.hpp"

#include <concepts>

namespace mimir::languages::dl::cnf_grammar
{

/**
 * Grammar constructor hierarchy parallel to dl constructors.
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class IConstructor
{
protected:
    IConstructor() = default;
    // Move constructor and move assignment operator are protected
    // to restrict their usage to derived classes only.
    IConstructor(IConstructor&& other) = default;
    IConstructor& operator=(IConstructor&& other) = default;

public:
    // Uncopieable
    IConstructor(const IConstructor& other) = delete;
    IConstructor& operator=(const IConstructor& other) = delete;

    virtual ~IConstructor() = default;

    virtual bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const = 0;

    /// @brief Accept a visitor.
    virtual void accept(IVisitor& visitor) const = 0;
};

}

#endif
