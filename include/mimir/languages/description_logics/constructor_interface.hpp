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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/denotations.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::languages::dl
{

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
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

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual Denotation<D> evaluate(EvaluationContext& context) const = 0;

    /// @brief Accept a visitor.
    virtual void accept(IVisitor& visitor) const = 0;

    /// @brief Return the index of the constructor.
    virtual Index get_index() const = 0;
};

}

#endif
